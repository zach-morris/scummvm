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

#ifndef KYRA_LOLEOBBASE_H
#define KYRA_LOLEOBBASE_H

#if defined(ENABLE_EOB) || defined(ENABLE_LOL)

#include "kyra/kyra_v1.h"
#include "kyra/screen_eob.h"
#include "kyra/gui_eob.h"
#include "kyra/text_lol.h"

namespace Kyra {

struct LevelDecorationProperty {
	uint16 shapeIndex[10];
	uint8 scaleFlag[10];
	int16 shapeX[10];
	int16 shapeY[10];
	int8 next;
	uint8 flags;
};

struct LevelBlockProperty {
	uint8 walls[4];
	uint16 assignedObjects;
	uint16 drawObjects;
	uint8 direction;
	uint16 flags;
};

struct OpenDoorState {
	uint16 block;
	int8 wall;
	int8 state;
};

struct LevelTempData {
	uint8 *wallsXorData;
	uint16 *flags;
	void *monsters;
	void *flyingObjects;
	uint8 monsterDifficulty;
};

struct EobFlyingObject {
	uint8 enable;
	uint8 objectType;
	int16 attackerId;
	Item item;	
	uint16 curBlock;
	uint16 u2;
	uint8 u1;
	uint8 direction;
	uint8 distance;
	int8 callBackIndex;
	uint8 curPos;
	uint8 flags;
	uint8 unused;
};

class LolEobBaseEngine : public KyraEngine_v1 {
friend class TextDisplayer_Eob;
public:
	LolEobBaseEngine(OSystem *system, const GameFlags &flags);
	virtual ~LolEobBaseEngine();

	virtual Screen *screen() = 0;
	virtual GUI_v1 *gui() const = 0;

protected:
	// Startup
	virtual Common::Error init();
	virtual Common::Error go() = 0;

	// Init
	void initStaticResource();

	const uint8 **_itemIconShapes;

	// Main loop
	virtual void update() = 0;
	void updateEnvironmentalSfx(int soundId);

	// timers
	virtual void setupTimers() = 0;
	void enableSysTimer(int sysTimer);
	void disableSysTimer(int sysTimer);
	void enableTimer(int id);
	virtual uint8 getClock2Timer(int index) = 0;
	virtual uint8 getNumClock2Timers() = 0;

	void timerProcessDoors(int timerNum);

	// mouse
	bool posWithinRect(int posX, int posY, int x1, int y1, int x2, int y2);
	virtual void setHandItem(Item itemIndex) = 0;

	// Characters
	int _updateCharNum;
	int _updatePortraitSpeechAnimDuration;
	bool _fadeText;
	int _resetPortraitAfterSpeechAnim;
	int _needSceneRestore;

	// Items
	int _itemInHand;

	// Monsters
	int getBlockDistance(uint16 block1, uint16 block2);

	uint8 **_monsterPalettes;
	uint8 **_monsterShapes;

	int16 _shpDmX1;
	int16 _shpDmX2;

	int _monsterStepCounter;
	int _monsterStepMode;

	// Level
	virtual void addLevelItems() = 0;
	virtual void loadBlockProperties(const char *file) = 0;

	virtual void drawScene(int pageNum) = 0;
	virtual void drawSceneShapes(int start) = 0;
	virtual void drawDecorations(int index) = 0;

	void setLevelShapesDim(int index, int16 &x1, int16 &x2, int dim);
	void scaleLevelShapesDim(int index, int16 &y1, int16 &y2, int dim);
	void drawLevelModifyScreenDim(int dim, int16 x1, int16 y1, int16 x2, int16 y2);
	void generateBlockDrawingBuffer();
	void generateVmpTileData(int16 startBlockX, uint8 startBlockY, uint8 wllVmpIndex, int16 vmpOffset, uint8 numBlocksX, uint8 numBlocksY);
	void generateVmpTileDataFlipped(int16 startBlockX, uint8 startBlockY, uint8 wllVmpIndex, int16 vmpOffset, uint8 numBlocksX, uint8 numBlocksY);
	bool hasWall(int index);
	void assignVisibleBlocks(int block, int direction);
	bool checkSceneUpdateNeed(int block);
	void drawVcnBlocks();
	uint16 calcNewBlockPosition(uint16 curBlock, uint16 direction);

	virtual int clickedDoorSwitch(uint16 block, uint16 direction) = 0;
	int clickedWallShape(uint16 block, uint16 direction);
	int clickedLeverOn(uint16 block, uint16 direction);
	int clickedLeverOff(uint16 block, uint16 direction);
	int clickedWallOnlyScript(uint16 block);
	virtual int clickedNiche(uint16 block, uint16 direction) = 0;

	void processDoorSwitch(uint16 block, int openClose);
	void openCloseDoor(int block, int openClose);
	void completeDoorOperations();

	uint8 *_wllVmpMap;
	int8 *_wllShapeMap;
	uint8 *_specialWallTypes;
	uint8 *_wllWallFlags;

	int _sceneXoffset;
	int _sceneShpDim;

	LevelBlockProperty *_levelBlockProperties;
	LevelBlockProperty *_visibleBlocks[18];
	LevelDecorationProperty *_levelDecorationData;
	uint16 _levelDecorationDataSize;
	LevelDecorationProperty *_levelDecorationProperties;
	uint8 **_levelDecorationShapes;
	uint16 _decorationCount;
	int16 _mappedDecorationsCount;
	uint16 *_vmpPtr;
	uint8 *_vcnBlocks;
	uint8 *_vcfBlocks;
	uint8 *_vcnShift;
	uint8 *_vcnExpTable;
	uint16 *_blockDrawingBuffer;
	uint8 *_sceneWindowBuffer;
	uint8 _blockBrightness;
	uint8 _wllVcnOffset;

	uint8 **_doorShapes;

	uint8 _currentLevel;
	uint16 _currentBlock;
	uint16 _currentDirection;
	int _sceneDefaultUpdate;
	bool _sceneUpdateRequired;

	int16 _visibleBlockIndex[18];
	int16 *_lvlShapeLeftRight;
	int16 *_lvlShapeTop;
	int16 *_lvlShapeBottom;

	char _lastBlockDataFile[13];
	uint32 _hasTempDataFlags;

	int16 _sceneDrawVarDown;
	int16 _sceneDrawVarRight;
	int16 _sceneDrawVarLeft;
	int _wllProcessFlag;

	OpenDoorState _openDoorState[3];

	int _sceneDrawPage1;
	int _sceneDrawPage2;

	const int8 *_dscShapeIndex;
	const uint8 *_dscDimMap;
	const int8 *_dscDim1;
	const int8 *_dscDim2;
	const int16 *_dscShapeX;
	const uint8 *_dscUnk2;
	const uint8 *_dscBlockMap;
	const int8 *_dscBlockIndex;
	const uint8 *_dscTileIndex;

	const uint8 *_dscDoorShpIndex;
	const uint8 *_dscDoorY2;

	// Script
	virtual void runLevelScript(int block, int flags) = 0;

	// Gui
	void removeInputTop();
	void gui_drawBox(int x, int y, int w, int h, int frameColor1, int frameColor2, int fillColor);
	virtual void gui_drawHorizontalBarGraph(int x, int y, int w, int h, int32 curVal, int32 maxVal, int col1, int col2);
	void gui_initButtonsFromList(const int16 *list);
	virtual void gui_initButton(int index, int x = -1, int y = -1, int val = -1) = 0;
	void gui_resetButtonList();
	void gui_notifyButtonListChanged();

	bool clickedShape(int shapeIndex);

	int _clickedShapeXOffs;
	int _clickedShapeYOffs;

	Button *_activeButtons;
	Button _activeButtonData[70];
	//bool _processingButtons;

	uint8 _mouseClick;
	bool _preserveEvents;
	bool _buttonListChanged;

	int _updateFlags;
	int _clickedSpecialFlag;

	int _compassDirection;

	static const uint8 _dropItemDirIndex[];

	// text
	void drawDialogueButtons();
	uint16 processDialogue();

	TextDisplayer_Eob *_txt;
	virtual TextDisplayer_Eob *txt() { return _txt; }

	bool _dialogueField;

	const char *_dialogueButtonString[9];
	const uint16 *_dialogueButtonPosX;
	const uint8 *_dialogueButtonPosY;
	int16 _dialogueButtonYoffs;
	uint16 _dialogueButtonW;
	uint16 _dialogueButtonH;
	int _dialogueNumButtons;
	int _dialogueHighlightedButton;
	int _currentControlMode;
	int _specialSceneFlag;
	uint8 _dialogueButtonLabelCol1;
	uint8 _dialogueButtonLabelCol2;

	int _bkgColor_1;
	uint8 _color1_1;
	uint8 _color2_1;

	const char *const *_moreStrings;

	// misc
	virtual void delay(uint32 millis, bool doUpdate = false, bool isMainLoop = false) = 0;
	void delayUntil(uint32 time, bool doUpdate = false, bool isMainLoop = false);
	int rollDice(int times, int pips, int inc = 0);

	virtual Common::Error loadGameState(int slot) = 0;
	virtual Common::Error saveGameStateIntern(int slot, const char *saveName, const Graphics::Surface *thumbnail) = 0;

	void generateTempData();
	void restoreBlockTempData(int levelIndex);
	void releaseTempData();
	virtual void *generateMonsterTempData(LevelTempData *tmp) = 0;
	virtual void restoreMonsterTempData(LevelTempData *tmp) = 0;
	virtual void releaseMonsterTempData(LevelTempData *tmp) = 0;
	void restoreFlyingObjectTempData(LevelTempData *tmp);
	void *generateFlyingObjectTempData(LevelTempData *tmp);
	void releaseFlyingObjectTempData(LevelTempData *tmp);

	LevelTempData *_lvlTempData[29];
	const int _numFlyingObjects;
	uint32 _flyingObjectStructSize;
	void *_flyingObjectsPtr;

	// sound
	virtual bool snd_processEnvironmentalSoundEffect(int soundId, int block);
	virtual void snd_stopSpeech(bool) {}
	virtual int snd_updateCharacterSpeech() { return 0; }
	virtual void stopPortraitSpeechAnim() {}
	virtual void setupOpcodeTable() {}
	virtual void snd_playVoiceFile(int) {}

	int _environmentSfx;
	int _environmentSfxVol;
	int _envSfxDistThreshold;

	uint32 _activeVoiceFileTotalTime;

	// unused
	void setWalkspeed(uint8) {}
	void removeHandItem() {}
	bool lineIsPassable(int, int) { return false; }
};

}	// End of namespace Kyra

#endif // ENABLE_EOB || ENABLE_LOL

#endif