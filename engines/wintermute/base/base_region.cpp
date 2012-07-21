/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

/*
 * This file is based on WME Lite.
 * http://dead-code.org/redir.php?target=wmelite
 * Copyright (c) 2011 Jan Nedoma
 */

#include "engines/wintermute/dcgf.h"
#include "engines/wintermute/base/base_region.h"
#include "engines/wintermute/base/base_parser.h"
#include "engines/wintermute/base/base_dynamic_buffer.h"
#include "engines/wintermute/base/base_game.h"
#include "engines/wintermute/base/scriptables/script.h"
#include "engines/wintermute/base/scriptables/script_stack.h"
#include "engines/wintermute/base/scriptables/script_value.h"
#include "engines/wintermute/base/base_file_manager.h"
#include "engines/wintermute/platform_osystem.h"
#include <limits.h>

namespace WinterMute {

IMPLEMENT_PERSISTENT(BaseRegion, false)

//////////////////////////////////////////////////////////////////////////
BaseRegion::BaseRegion(BaseGame *inGame): BaseObject(inGame) {
	_active = true;
	_editorSelectedPoint = -1;
	_lastMimicScale = -1;
	_lastMimicX = _lastMimicY = INT_MIN;

	BasePlatform::setRectEmpty(&_rect);
}


//////////////////////////////////////////////////////////////////////////
BaseRegion::~BaseRegion() {
	cleanup();
}


//////////////////////////////////////////////////////////////////////////
void BaseRegion::cleanup() {
	for (int i = 0; i < _points.getSize(); i++) delete _points[i];
	_points.removeAll();

	BasePlatform::setRectEmpty(&_rect);
	_editorSelectedPoint = -1;
}


//////////////////////////////////////////////////////////////////////////
bool BaseRegion::createRegion() {
	return DID_SUCCEED(getBoundingRect(&_rect));
}


//////////////////////////////////////////////////////////////////////////
bool BaseRegion::pointInRegion(int x, int y) {
	if (_points.getSize() < 3) return false;

	Point32 pt;
	pt.x = x;
	pt.y = y;

	Rect32 rect;
	rect.left = x - 1;
	rect.right = x + 2;
	rect.top = y - 1;
	rect.bottom = y + 2;

	if (BasePlatform::ptInRect(&_rect, pt)) return ptInPolygon(x, y);
	else return false;
}


//////////////////////////////////////////////////////////////////////////
bool BaseRegion::loadFile(const char *filename) {
	byte *buffer = _gameRef->_fileManager->readWholeFile(filename);
	if (buffer == NULL) {
		_gameRef->LOG(0, "BaseRegion::LoadFile failed for file '%s'", filename);
		return STATUS_FAILED;
	}

	bool ret;

	_filename = new char [strlen(filename) + 1];
	strcpy(_filename, filename);

	if (DID_FAIL(ret = loadBuffer(buffer, true))) _gameRef->LOG(0, "Error parsing REGION file '%s'", filename);


	delete [] buffer;

	return ret;
}


TOKEN_DEF_START
TOKEN_DEF(REGION)
TOKEN_DEF(TEMPLATE)
TOKEN_DEF(NAME)
TOKEN_DEF(ACTIVE)
TOKEN_DEF(POINT)
TOKEN_DEF(CAPTION)
TOKEN_DEF(SCRIPT)
TOKEN_DEF(EDITOR_SELECTED_POINT)
TOKEN_DEF(PROPERTY)
TOKEN_DEF_END
//////////////////////////////////////////////////////////////////////////
bool BaseRegion::loadBuffer(byte *buffer, bool complete) {
	TOKEN_TABLE_START(commands)
	TOKEN_TABLE(REGION)
	TOKEN_TABLE(TEMPLATE)
	TOKEN_TABLE(NAME)
	TOKEN_TABLE(ACTIVE)
	TOKEN_TABLE(POINT)
	TOKEN_TABLE(CAPTION)
	TOKEN_TABLE(SCRIPT)
	TOKEN_TABLE(EDITOR_SELECTED_POINT)
	TOKEN_TABLE(PROPERTY)
	TOKEN_TABLE_END

	byte *params;
	int cmd;
	BaseParser parser(_gameRef);

	if (complete) {
		if (parser.getCommand((char **)&buffer, commands, (char **)&params) != TOKEN_REGION) {
			_gameRef->LOG(0, "'REGION' keyword expected.");
			return STATUS_FAILED;
		}
		buffer = params;
	}

	int i;

	for (i = 0; i < _points.getSize(); i++) delete _points[i];
	_points.removeAll();

	while ((cmd = parser.getCommand((char **)&buffer, commands, (char **)&params)) > 0) {
		switch (cmd) {
		case TOKEN_TEMPLATE:
			if (DID_FAIL(loadFile((char *)params))) cmd = PARSERR_GENERIC;
			break;

		case TOKEN_NAME:
			setName((char *)params);
			break;

		case TOKEN_CAPTION:
			setCaption((char *)params);
			break;

		case TOKEN_ACTIVE:
			parser.scanStr((char *)params, "%b", &_active);
			break;

		case TOKEN_POINT: {
			int x, y;
			parser.scanStr((char *)params, "%d,%d", &x, &y);
			_points.add(new BasePoint(x, y));
		}
		break;

		case TOKEN_SCRIPT:
			addScript((char *)params);
			break;

		case TOKEN_EDITOR_SELECTED_POINT:
			parser.scanStr((char *)params, "%d", &_editorSelectedPoint);
			break;

		case TOKEN_PROPERTY:
			parseProperty(params, false);
			break;
		}
	}
	if (cmd == PARSERR_TOKENNOTFOUND) {
		_gameRef->LOG(0, "Syntax error in REGION definition");
		return STATUS_FAILED;
	}

	createRegion();

	return STATUS_OK;
}


//////////////////////////////////////////////////////////////////////////
// high level scripting interface
//////////////////////////////////////////////////////////////////////////
bool BaseRegion::scCallMethod(ScScript *script, ScStack *stack, ScStack *thisStack, const char *name) {

	//////////////////////////////////////////////////////////////////////////
	// AddPoint
	//////////////////////////////////////////////////////////////////////////
	if (strcmp(name, "AddPoint") == 0) {
		stack->correctParams(2);
		int x = stack->pop()->getInt();
		int y = stack->pop()->getInt();

		_points.add(new BasePoint(x, y));
		createRegion();

		stack->pushBool(true);

		return STATUS_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// InsertPoint
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "InsertPoint") == 0) {
		stack->correctParams(3);
		int Index = stack->pop()->getInt();
		int x = stack->pop()->getInt();
		int y = stack->pop()->getInt();

		if (Index >= 0 && Index < _points.getSize()) {
			_points.insertAt(Index, new BasePoint(x, y));
			createRegion();

			stack->pushBool(true);
		} else stack->pushBool(false);

		return STATUS_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// SetPoint
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "SetPoint") == 0) {
		stack->correctParams(3);
		int Index = stack->pop()->getInt();
		int x = stack->pop()->getInt();
		int y = stack->pop()->getInt();

		if (Index >= 0 && Index < _points.getSize()) {
			_points[Index]->x = x;
			_points[Index]->y = y;
			createRegion();

			stack->pushBool(true);
		} else stack->pushBool(false);

		return STATUS_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// RemovePoint
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "RemovePoint") == 0) {
		stack->correctParams(1);
		int index = stack->pop()->getInt();

		if (index >= 0 && index < _points.getSize()) {
			delete _points[index];
			_points[index] = NULL;

			_points.removeAt(index);
			createRegion();

			stack->pushBool(true);
		} else stack->pushBool(false);

		return STATUS_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// GetPoint
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "GetPoint") == 0) {
		stack->correctParams(1);
		int index = stack->pop()->getInt();

		if (index >= 0 && index < _points.getSize()) {
			ScValue *val = stack->getPushValue();
			if (val) {
				val->setProperty("X", _points[index]->x);
				val->setProperty("Y", _points[index]->y);
			}
		} else stack->pushNULL();

		return STATUS_OK;
	}

	else return BaseObject::scCallMethod(script, stack, thisStack, name);
}


//////////////////////////////////////////////////////////////////////////
ScValue *BaseRegion::scGetProperty(const char *name) {
	_scValue->setNULL();

	//////////////////////////////////////////////////////////////////////////
	// Type
	//////////////////////////////////////////////////////////////////////////
	if (strcmp(name, "Type") == 0) {
		_scValue->setString("region");
		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// Name
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "Name") == 0) {
		_scValue->setString(_name);
		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// Active
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "Active") == 0) {
		_scValue->setBool(_active);
		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// NumPoints
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "NumPoints") == 0) {
		_scValue->setInt(_points.getSize());
		return _scValue;
	}

	else return BaseObject::scGetProperty(name);
}


//////////////////////////////////////////////////////////////////////////
bool BaseRegion::scSetProperty(const char *name, ScValue *value) {
	//////////////////////////////////////////////////////////////////////////
	// Name
	//////////////////////////////////////////////////////////////////////////
	if (strcmp(name, "Name") == 0) {
		setName(value->getString());
		return STATUS_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// Active
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "Active") == 0) {
		_active = value->getBool();
		return STATUS_OK;
	}

	else return BaseObject::scSetProperty(name, value);
}


//////////////////////////////////////////////////////////////////////////
const char *BaseRegion::scToString() {
	return "[region]";
}


//////////////////////////////////////////////////////////////////////////
bool BaseRegion::saveAsText(BaseDynamicBuffer *buffer, int indent, const char *nameOverride) {
	if (!nameOverride) buffer->putTextIndent(indent, "REGION {\n");
	else buffer->putTextIndent(indent, "%s {\n", nameOverride);

	buffer->putTextIndent(indent + 2, "NAME=\"%s\"\n", _name);
	buffer->putTextIndent(indent + 2, "CAPTION=\"%s\"\n", getCaption());
	buffer->putTextIndent(indent + 2, "ACTIVE=%s\n", _active ? "TRUE" : "FALSE");
	buffer->putTextIndent(indent + 2, "EDITOR_SELECTED_POINT=%d\n", _editorSelectedPoint);

	int i;

	for (i = 0; i < _scripts.getSize(); i++) {
		buffer->putTextIndent(indent + 2, "SCRIPT=\"%s\"\n", _scripts[i]->_filename);
	}

	for (i = 0; i < _points.getSize(); i++) {
		buffer->putTextIndent(indent + 2, "POINT {%d,%d}\n", _points[i]->x, _points[i]->y);
	}

	if (_scProp) _scProp->saveAsText(buffer, indent + 2);

	buffer->putTextIndent(indent, "}\n\n");

	return STATUS_OK;
}


//////////////////////////////////////////////////////////////////////////
bool BaseRegion::persist(BasePersistenceManager *persistMgr) {

	BaseObject::persist(persistMgr);

	persistMgr->transfer(TMEMBER(_active));
	persistMgr->transfer(TMEMBER(_editorSelectedPoint));
	persistMgr->transfer(TMEMBER(_lastMimicScale));
	persistMgr->transfer(TMEMBER(_lastMimicX));
	persistMgr->transfer(TMEMBER(_lastMimicY));
	_points.persist(persistMgr);

	return STATUS_OK;
}


typedef struct {
	double x, y;
} dPoint;

//////////////////////////////////////////////////////////////////////////
bool BaseRegion::ptInPolygon(int x, int y) {
	if (_points.getSize() < 3) return false;

	int counter = 0;
	double xinters;
	dPoint p, p1, p2;

	p.x = (double)x;
	p.y = (double)y;

	p1.x = (double)_points[0]->x;
	p1.y = (double)_points[0]->y;

	for (int i = 1; i <= _points.getSize(); i++) {
		p2.x = (double)_points[i % _points.getSize()]->x;
		p2.y = (double)_points[i % _points.getSize()]->y;

		if (p.y > MIN(p1.y, p2.y)) {
			if (p.y <= MAX(p1.y, p2.y)) {
				if (p.x <= MAX(p1.x, p2.x)) {
					if (p1.y != p2.y) {
						xinters = (p.y - p1.y) * (p2.x - p1.x) / (p2.y - p1.y) + p1.x;
						if (p1.x == p2.x || p.x <= xinters)
							counter++;
					}
				}
			}
		}
		p1 = p2;
	}

	if (counter % 2 == 0)
		return false;
	else
		return true;
}


//////////////////////////////////////////////////////////////////////////
bool BaseRegion::getBoundingRect(Rect32 *rect) {
	if (_points.getSize() == 0) BasePlatform::setRectEmpty(rect);
	else {
		int MinX = INT_MAX, MinY = INT_MAX, MaxX = INT_MIN, MaxY = INT_MIN;

		for (int i = 0; i < _points.getSize(); i++) {
			MinX = MIN(MinX, _points[i]->x);
			MinY = MIN(MinY, _points[i]->y);

			MaxX = MAX(MaxX, _points[i]->x);
			MaxY = MAX(MaxY, _points[i]->y);
		}
		BasePlatform::setRect(rect, MinX, MinY, MaxX, MaxY);
	}
	return STATUS_OK;
}


//////////////////////////////////////////////////////////////////////////
bool BaseRegion::mimic(BaseRegion *region, float scale, int x, int y) {
	if (scale == _lastMimicScale && x == _lastMimicX && y == _lastMimicY) return STATUS_OK;

	cleanup();

	for (int i = 0; i < region->_points.getSize(); i++) {
		int xVal, yVal;

		xVal = (int)((float)region->_points[i]->x * scale / 100.0f);
		yVal = (int)((float)region->_points[i]->y * scale / 100.0f);

		_points.add(new BasePoint(xVal + x, yVal + y));
	}

	_lastMimicScale = scale;
	_lastMimicX = x;
	_lastMimicY = y;

	return createRegion() ? STATUS_OK : STATUS_FAILED;
}

} // end of namespace WinterMute
