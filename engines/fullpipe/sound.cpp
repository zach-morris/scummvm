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

#include "fullpipe/fullpipe.h"

#include "fullpipe/objects.h"
#include "fullpipe/ngiarchive.h"

namespace Fullpipe {

SoundList::SoundList() {
	_soundItems = 0;
	_soundItemsCount = 0;
	_libHandle = 0;
}

bool SoundList::load(MfcArchive &file, char *fname) {
	_soundItemsCount = file.readUint32LE();
	_soundItems = (Sound **)calloc(_soundItemsCount, sizeof(Sound *));

	if (fname) {
	  _libHandle = (NGIArchive *)makeNGIArchive(fname);
	} else {
		_libHandle = 0;
	}

	for (int i = 0; i < _soundItemsCount; i++) {
		Sound *snd = new Sound();

		_soundItems[i] = 0;
		snd->load(file, _libHandle);
	}

	return true;
	
}

bool SoundList::loadFile(const char *fname, char *libname) {
	Common::File file;

	if (!file.open(fname))
		return false;

	MfcArchive archive(&file);

	return load(archive, libname);
}

Sound::Sound() {
	_id = 0;
	_directSoundBuffer = 0;
	_soundData = 0;
	_objectId = 0;
	 memset(_directSoundBuffers, 0, sizeof(_directSoundBuffers));
}


bool Sound::load(MfcArchive &file, NGIArchive *archive) {
	MemoryObject::load(file);

	_id = file.readUint32LE();
	_description = file.readPascalString();

	assert(g_fullpipe->_gameProjectVersion >= 6);

	_objectId = file.readUint16LE();

	if (archive && archive->hasFile(_filename)) {
		Common::SeekableReadStream *s = archive->createReadStreamForMember(_filename);

		_soundData = (byte *)calloc(s->size(), 1);

		s->read(_soundData, s->size());

		delete s;
	}

	return true;
}

} // End of namespace Fullpipe
