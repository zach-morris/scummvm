/* ScummVM - Scumm Interpreter
 * Copyright (C) 2001  Ludvig Strigeus
 * Copyright (C) 2001-2004 The ScummVM project
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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * $Header$
 *
 */

#include "stdafx.h"

#include "common/config-manager.h"

#if defined(UNIX)
#include <sys/param.h>
#ifndef MAXPATHLEN
#define MAXPATHLEN 256
#endif
#ifdef MACOSX
#define DEFAULT_CONFIG_FILE "Library/Preferences/ScummVM Preferences"
#else
#define DEFAULT_CONFIG_FILE ".scummvmrc"
#endif
#else
#define DEFAULT_CONFIG_FILE "scummvm.ini"
#endif

#define MAXLINELEN 256

static char *ltrim(char *t) {
	while (isspace(*t))
		t++;
	return t;
}

static char *rtrim(char *t) {
	int l = strlen(t) - 1;
	while (l >= 0 && isspace(t[l]))
		t[l--] = 0;
	return t;
}

namespace Common {

const String ConfigManager::kApplicationDomain("scummvm");
const String ConfigManager::kTransientDomain("__TRANSIENT");

const String trueStr("true");
const String falseStr("false");


#pragma mark -


ConfigManager::ConfigManager() {

#if defined(UNIX)
	char configFile[MAXPATHLEN];
	if(getenv("HOME") != NULL)
		sprintf(configFile,"%s/%s", getenv("HOME"), DEFAULT_CONFIG_FILE);
	else strcpy(configFile,DEFAULT_CONFIG_FILE);
#else
	char configFile[256];
	#if defined (WIN32) && !defined(_WIN32_WCE)
		GetWindowsDirectory(configFile, 256);
		strcat(configFile, "\\");
		strcat(configFile, DEFAULT_CONFIG_FILE);
	#elif defined(__PALM_OS__)
		strcpy(configFile,"/PALM/Programs/ScummVM/");
		strcat(configFile, DEFAULT_CONFIG_FILE);
	#else
		strcpy(configFile, DEFAULT_CONFIG_FILE);
	#endif
#endif

	switchFile(configFile);
}

void ConfigManager::switchFile(const String &filename) {
	_globalDomains.clear();
	_gameDomains.clear();
	_transientDomain.clear();

	// Ensure the global domain(s) are setup.
	_globalDomains.addKey(kApplicationDomain);
#ifdef _WIN32_WCE
	// WinCE for some reasons uses additional global domains.
	_globalDomains.addKey("wince");
	_globalDomains.addKey("smartfon-keys");
#endif

	_filename = filename;
	_domainSaveOrder.clear();
	loadFile(_filename);
	debug(1, "Switched to configuration %s", _filename.c_str());
}

void ConfigManager::loadFile(const String &filename) {
	FILE *cfg_file;

	if (!(cfg_file = fopen(filename.c_str(), "r"))) {
		warning("Unable to open configuration file: %s", filename.c_str());
	} else {
		char buf[MAXLINELEN];
		String domain;
		String comment;
		int lineno = 0;
		
		// TODO: Detect if a domain occurs multiple times (or likewise, if
		// a key occurs multiple times inside one domain).

		while (!feof(cfg_file)) {
			lineno++;
			if (!fgets(buf, MAXLINELEN, cfg_file))
				continue;

			if (buf[0] == '#') {
				// Accumulate comments here. Once we encounter either the start
				// of a new domain, or a key-value-pair, we associate the value
				// of the 'comment' variable with that entity.
				comment += buf;
			} else if (buf[0] == '[') {
				// It's a new domain which begins here.
				char *p = buf + 1;
				// Get the domain name, and check whether it's valid (that
				// is, verify that it only consists of alphanumerics,
				// dashes and underscores).
				while (*p && (isalnum(*p) || *p == '-' || *p == '_'))
					p++;

				switch (*p) {
				case '\0':
					error("Config file buggy: missing ] in line %d", lineno);
					break;
				case ']':
					*p = 0;
					domain = buf + 1;
					break;
				default:
					error("Config file buggy: Invalid character '%c' occured in domain name in line %d", *p, lineno);
				}
				
				// Store domain comment
				if (_globalDomains.contains(domain)) {
					_globalDomains[domain].setDomainComment(comment);
				} else {
					_gameDomains[domain].setDomainComment(comment);
				}
				comment.clear();
				
				_domainSaveOrder.push_back(domain);
			} else {
				// Skip leading & trailing whitespaces
				char *t = rtrim(ltrim(buf));

				// Skip empty lines
				if (*t == 0)
					continue;

				// If no domain has been set, this config file is invalid!
				if (domain.isEmpty()) {
					error("Config file buggy: Key/value pair found outside a domain in line %d", lineno);
				}

				// Split string at '=' into 'key' and 'value'.
				char *p = strchr(t, '=');
				if (!p)
					error("Config file buggy: Junk found in line line %d: '%s'", lineno, t);
				*p = 0;
				String key = rtrim(t);
				String value = ltrim(p + 1);
				set(key, value, domain);

				// Store comment
				if (_globalDomains.contains(domain)) {
					_globalDomains[domain].setKVComment(key, comment);
				} else {
					_gameDomains[domain].setKVComment(key, comment);
				}
				comment.clear();
			}
		}
		fclose(cfg_file);
	}
}

void ConfigManager::flushToDisk() {
	FILE *cfg_file;

// TODO
//	if (!willwrite)
//		return;

	if (!(cfg_file = fopen(_filename.c_str(), "w"))) {
		warning("Unable to write configuration file: %s", _filename.c_str());
	} else {
		
		// First write the domains in _domainSaveOrder, in that order.
		// Note: It's possible for _domainSaveOrder to list domains which
		// are not present anymore.
		StringList::const_iterator i;
		for (i = _domainSaveOrder.begin(); i != _domainSaveOrder.end(); ++i) {
			if (_globalDomains.contains(*i)) {
				writeDomain(cfg_file, *i, _globalDomains[*i]);
			} else if (_gameDomains.contains(*i)) {
				writeDomain(cfg_file, *i, _gameDomains[*i]);
			}
		}

		DomainMap::const_iterator d;

		// Now write the global domains which weren't written yet
		for (d = _globalDomains.begin(); d != _globalDomains.end(); ++d) {
			if (!_domainSaveOrder.contains(d->_key))
				writeDomain(cfg_file, d->_key, d->_value);
		}
		
		// Finally write the remaining game domains
		for (d = _gameDomains.begin(); d != _gameDomains.end(); ++d) {
			if (!_domainSaveOrder.contains(d->_key))
				writeDomain(cfg_file, d->_key, d->_value);
		}

		fclose(cfg_file);
	}
}

void ConfigManager::writeDomain(FILE *file, const String &name, const Domain &domain) {
	if (domain.isEmpty())
		return;		// Don't bother writing empty domains.
	
	String comment;
	
	// Write domain comment (if any)
	comment = domain.getDomainComment();
	if (!comment.isEmpty())
		fprintf(file, "%s", comment.c_str());
	
	// Write domain start
	fprintf(file, "[%s]\n", name.c_str());

	// Write all key/value pairs in this domain, including comments
	Domain::const_iterator x;
	for (x = domain.begin(); x != domain.end(); ++x) {
		const String &value = x->_value;
		if (!value.isEmpty()) {
			// Write comment (if any)
			if (domain.hasKVComment(x->_key)) {
				comment = domain.getKVComment(x->_key);
				fprintf(file, "%s", comment.c_str());
			}
			// Write the key/value pair
			fprintf(file, "%s=%s\n", x->_key.c_str(), value.c_str());
		}
	}
	fprintf(file, "\n");
}

#pragma mark -


bool ConfigManager::hasKey(const String &key) const {
	// Search the domains in the following order:
	// 1) Transient domain
	// 2) Active game domain (if any)
	// 3) All global domains
	// The defaults domain is explicitly *not* checked.
	
	if (_transientDomain.contains(key))
		return true;

	if (!_activeDomain.isEmpty() && _gameDomains[_activeDomain].contains(key))
		return true;
	
	DomainMap::const_iterator iter;
	for (iter = _globalDomains.begin(); iter != _globalDomains.end(); ++iter) {
		if (iter->_value.contains(key))
			return true;
	}

	return false;
}

bool ConfigManager::hasKey(const String &key, const String &dom) const {
	assert(!dom.isEmpty());

	if (dom == kTransientDomain)
		return _transientDomain.contains(key);
	if (_gameDomains.contains(dom))
		return _gameDomains[dom].contains(key);
	if (_globalDomains.contains(dom))
		return _globalDomains[dom].contains(key);
	
	return false;
}

void ConfigManager::removeKey(const String &key, const String &dom) {
	assert(!dom.isEmpty());

	if (dom == kTransientDomain)
		_transientDomain.remove(key);
	else if (_gameDomains.contains(dom))
		_gameDomains[dom].remove(key);
	else if (_globalDomains.contains(dom))
		_globalDomains[dom].remove(key);
	else
		error("Removing key '%s' from non-existent domain '%s'", key.c_str(), dom.c_str());
}


#pragma mark -


const String & ConfigManager::get(const String &key, const String &domain) const {
	// Search the domains in the following order:
	// 1) Transient domain
	// 2) Active game domain (if any)
	// 3) All global domains
	// 4) The defaults 


	if ((domain.isEmpty() || domain == kTransientDomain) && _transientDomain.contains(key))
		return _transientDomain[key];

	const String &dom = domain.isEmpty() ? _activeDomain : domain;

	if (!dom.isEmpty() && _gameDomains.contains(dom) && _gameDomains[dom].contains(key))
		return _gameDomains[dom][key];

	DomainMap::const_iterator iter;
	for (iter = _globalDomains.begin(); iter != _globalDomains.end(); ++iter) {
		if (iter->_value.contains(key))
			return iter->_value[key];
	}

	return _defaultsDomain.get(key);
}

int ConfigManager::getInt(const String &key, const String &dom) const {
	String value(get(key, dom));
	char *errpos;
	
	// For now, be tolerant against missing config keys. Strictly spoken, it is
	// a bug in the calling code to retrieve an int for a key which isn't even
	// present... and a default value of 0 seems rather arbitrary.
	if (value.isEmpty())
		return 0;

	int ivalue = (int)strtol(value.c_str(), &errpos, 10);
	if (value.c_str() == errpos)
		error("Config file buggy: '%s' is not a valid integer", errpos);

	return ivalue;
}

bool ConfigManager::getBool(const String &key, const String &dom) const {
	String value(get(key, dom));

	if ((value == trueStr) || (value == "yes") || (value == "1"))
		return true;
	if ((value == falseStr) || (value == "no") || (value == "0"))
		return false;

	error("Config file buggy: '%s' is not a valid bool", value.c_str());
}


#pragma mark -


void ConfigManager::set(const String &key, const String &value, const String &dom) {
	if (dom.isEmpty()) {
		// Remove the transient domain value
		_transientDomain.remove(key);
	
		if (_activeDomain.isEmpty())
			_globalDomains[kApplicationDomain][key] = value;
		else
			_gameDomains[_activeDomain][key] = value;

	} else {

		if (dom == kTransientDomain)
			_transientDomain[key] = value;
		else {
			if (_globalDomains.contains(dom)) {
				_globalDomains[dom][key] = value;
				if (_activeDomain.isEmpty() || !_gameDomains[_activeDomain].contains(key))
					_transientDomain.remove(key);
			} else {
				_gameDomains[dom][key] = value;
				if (dom == _activeDomain)
					_transientDomain.remove(key);
			}
		}
	}
}

void ConfigManager::set(const String &key, const char *value, const String &dom) {
	set(key, String(value), dom);
}

void ConfigManager::set(const String &key, int value, const String &dom) {
	char tmp[128];
	snprintf(tmp, sizeof(tmp), "%i", value);
	set(key, String(tmp), dom);
}

void ConfigManager::set(const String &key, bool value, const String &dom) {
	set(key, value ? trueStr : falseStr, dom);
}


#pragma mark -


void ConfigManager::registerDefault(const String &key, const String &value) {
	_defaultsDomain[key] = value;
}

void ConfigManager::registerDefault(const String &key, const char *value) {
	registerDefault(key, String(value));
}

void ConfigManager::registerDefault(const String &key, int value) {
	char tmp[128];
	snprintf(tmp, sizeof(tmp), "%i", value);
	registerDefault(key, tmp);
}

void ConfigManager::registerDefault(const String &key, bool value) {
	registerDefault(key, value ? trueStr : falseStr);
}


#pragma mark -


void ConfigManager::setActiveDomain(const String &domain) {
	assert(!domain.isEmpty());
	_activeDomain = domain;
	_gameDomains.addKey(domain);
}

void ConfigManager::removeGameDomain(const String &domain) {
	assert(!domain.isEmpty());
	_gameDomains.remove(domain);
}

void ConfigManager::renameGameDomain(const String &oldName, const String &newName) {
	if (oldName == newName)
		return;

	assert(!oldName.isEmpty());
	assert(!newName.isEmpty());

	_gameDomains[newName].merge(_gameDomains[oldName]);
	
	_gameDomains.remove(oldName);
}

bool ConfigManager::hasGameDomain(const String &domain) const {
	assert(!domain.isEmpty());
	return _gameDomains.contains(domain);
}


#pragma mark -


const String &ConfigManager::Domain::get(const String &key) const {
	Node *node = findNode(_root, key);
	return node ? node->_value : String::emptyString;
}

void ConfigManager::Domain::setDomainComment(const String &comment) {
	_domainComment = comment;
}
const String &ConfigManager::Domain::getDomainComment() const {
	return _domainComment;
}

void ConfigManager::Domain::setKVComment(const String &key, const String &comment) {
	_keyValueComments[key] = comment;
}
const String &ConfigManager::Domain::getKVComment(const String &key) const {
	return _keyValueComments[key];
}
bool ConfigManager::Domain::hasKVComment(const String &key) const {
	return _keyValueComments.contains(key);
}

}	// End of namespace Common
