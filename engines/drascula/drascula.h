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
 * $URL$
 * $Id$
 *
 */

#ifndef DRASCULA_H
#define DRASCULA_H

#include "common/scummsys.h"
#include "common/endian.h"
#include "common/util.h"
#include "common/file.h"
#include "common/savefile.h"
#include "common/system.h"
#include "common/hash-str.h"
#include "common/events.h"
#include "common/keyboard.h"
#include "common/unarj.h"

#include "sound/audiostream.h"
#include "sound/mixer.h"
#include "sound/voc.h"
#include "sound/audiocd.h"

#include "engines/engine.h"

namespace Drascula {

enum DrasculaGameFeatures {
	GF_PACKED = (1 << 0)
};

enum Languages {
	kEnglish = 0,
	kSpanish = 1,
	kGerman = 2,
	kFrench = 3,
	kItalian = 4
};

enum Verbs {
	kVerbDefault = -1,
	kVerbLook = 1,
	kVerbPick = 2,
	kVerbOpen = 3,
	kVerbClose = 4,
	kVerbTalk = 5,
	kVerbMove = 6
};

enum Colors {
	kColorBrown = 1,
	kColorDarkBlue = 2,
	kColorLightGreen = 3,
	kColorDarkGreen = 4,
	kColorYellow = 5,
	kColorOrange = 6,
	kColorRed = 7,
	kColorMaroon = 8,
	kColorPurple = 9,
	kColorWhite = 10,
	kColorPink = 11
};

enum SSNFrames {
	kFrameInit = 0,
	kFrameCmpRle = 1,
	kFrameCmpOff = 2,
	kFrameEndAnim = 3,
	kFrameSetPal = 4,
	kFrameMouseKey = 5,		// unused
	kFrameEmptyFrame = 6
};

enum IgorTalkerTypes {
	kIgorDch = 0,
	kIgorFront = 1,
	kIgorDoor = 2,
	kIgorSeated = 3,
	kIgorWig = 4
};

#define TEXTD_START 68

struct DrasculaGameDescription;

struct RoomTalkAction {
	int chapter;
	int action;
	int objectID;
	int speechID;
};

struct ItemLocation {
	int x;
	int y;
};

struct CharInfo {
	int inChar;
	int mappedChar;
	int charType;	// 0 - letters, 1 - signs, 2 - accented
};

#define CHARMAP_SIZE 93
#define NUM_SAVES     10
#define NUM_FLAGS     50
#define DIF_MASK       55
#define OBJWIDTH        40
#define OBJHEIGHT         25

#define DIF_MASK_HARE   72
#define DIF_MASK_ABC    22
#define CHAR_WIDTH     8
#define CHAR_HEIGHT      6

#define TALK_HEIGHT  25
#define TALK_WIDTH 23
#define STEP_X       8
#define STEP_Y       3
#define CHARACTER_HEIGHT   70
#define CHARACTER_WIDTH  43
#define FEET_HEIGHT        12

#define CHAR_WIDTH_OPC     6
#define CHAR_HEIGHT_OPC      5
#define NO_DOOR              99

#define COMPLETE_PAL   256
#define HALF_PAL       128

static const int interf_x[] ={ 1, 65, 129, 193, 1, 65, 129 };
static const int interf_y[] ={ 51, 51, 51, 51, 83, 83, 83 };

class DrasculaEngine : public ::Engine {
	Common::KeyState _keyPressed;

protected:
	int init();
	int go();
//	void shutdown();

public:
	DrasculaEngine(OSystem *syst, const DrasculaGameDescription *gameDesc);
	virtual ~DrasculaEngine();

	Common::RandomSource *_rnd;
	const DrasculaGameDescription *_gameDescription;
	uint32 getFeatures() const;
	Common::Language getLanguage() const;
	void updateEvents();

	void loadArchives();

	Audio::SoundHandle _soundHandle;

	void allocMemory();
	void freeMemory();
	void releaseGame();

	void loadPic(const char *NamePcc, byte *targetSurface, int colorCount);
	void decompressPic(byte *targetSurface, int colorCount);

	typedef char DacPalette256[256][3];

	void setRGB(byte *dir_lectura, int plt);
	void paleta_hare();
	void updatePalette();
	void setPalette(byte *PalBuf);
	void copyBackground(int xorg, int yorg, int xdes, int ydes, int width,
				int height, byte *src, byte *dest);
	void copyRect(int xorg, int yorg, int xdes, int ydes, int width,
				int height, byte *src, byte *dest);
	void copyRectClip(int *Array, byte *src, byte *dest);
	void updateScreen() {
		updateScreen(0, 0, 0, 0, 320, 200, screenSurface);
	}
	void updateScreen(int xorg, int yorg, int xdes, int ydes, int width, int height, byte *buffer);
	int checkWrapX(int x) {
		if (x < 0) x += 320;
		if (x > 319) x -= 320;
		return x;
	}
	int checkWrapY(int y) {
		if (y < 0) y += 200;
		if (y > 199) y -= 200;
		return y;
	}

	DacPalette256 gamePalette;
	DacPalette256 palHare;
	DacPalette256 palHareClaro;
	DacPalette256 palHareOscuro;

	byte *VGA;

	byte *drawSurface1;
	byte *backSurface;
	byte *drawSurface3;
	byte *drawSurface2;
	byte *tableSurface;
	byte *extraSurface;	// not sure about this one, was "dir_hare_dch"
	byte *screenSurface;
	byte *frontSurface;
	byte *textSurface;
	byte *pendulumSurface;

	byte cPal[768];
	byte *pcxBuffer;

	Common::ArjFile _arj;

	int hay_sb;
	int nivel_osc, previousMusic, roomMusic;
	int roomNumber;
	char roomDisk[20];
	char currentData[20];
	int numRoomObjs;
	char menuBackground[20];

	char objName[30][20];
	char iconName[44][13];

	int objectNum[40], visible[40], isDoor[40];
	int roomObjX[40], roomObjY[40], trackObj[40];
	int inventoryObjects[43];
	char _targetSurface[40][20];
	int _destX[40], _destY[40], sentido_alkeva[40], alapuertakeva[40];
	int x1[40], y1[40], x2[40], y2[40];
	int takeObject, pickedObject;
	int withVoices;
	int menuBar, menuScreen, hasName;
	char textName[20];
	int frame_blind;
	int frame_snore;
	int frame_bat;
	int c_mirar;
	int c_poder;

	int flags[NUM_FLAGS];

	int frame_y;
	int curX, curY, characterMoved, curDirection, trackProtagonist, num_frame, hare_se_ve;
	int roomX, roomY, checkFlags;
	int doBreak;
	int stepX, stepY;
	int curHeight, curWidth, feetHeight;
	int talkHeight, talkWidth;
	int floorX1, floorY1, floorX2, floorY2;
	int near, far;
	int trackFinal, walkToObject;
	int objExit;
	int timeDiff, startTime;
	int hasAnswer;
	int conta_blind_vez;
	int changeColor;
	int breakOut;
	int vbX, trackVB, vbHasMoved, frame_vb;
	float newHeight, newWidth;
	int factor_red[202];
	int frame_piano;
	int frame_drunk;
	int frame_candles;
	int color_solo;
	int blinking;
	int igorX, igorY, sentido_igor;
	int x_dr, y_dr, trackDrascula;
	int x_bj, y_bj, sentido_bj;
	int cont_sv;
	int term_int;
	int currentChapter;
	int hay_que_load;
	char saveName[13];
	int _color;
	int musicStopped;
	char select[23];
	int selectionMade;
	int mouseX;
	int mouseY;
	int mouseY_ant;
	int button_izq;
	int button_dch;

	bool escoba();
	void black();
	void pickObject(int);
	void walkUp();
	void walkDown();
	void moveVB();
	void lleva_vb(int pointX);
	void hipo_sin_nadie(int counter);
	void openDoor(int nflag, int doorNum);
	void showMap();

	void hare_oscuro();

	void withoutVerb();
	bool para_cargar(char[]);
	void carga_escoba(const char *);
	void clearRoom();
	void gotoObject(int, int);
	void moveCursor();
	void checkObjects();
	void selectVerbFromBar();
	bool verify1();
	bool verify2();
	Common::KeyCode getScan();
	void selectVerb(int);
	void mesa();
	bool saves();
	void print_abc(const char *, int, int);
	void delay(int ms);
	bool confirmExit();
	void screenSaver();
	void chooseObject(int objeto);
	void addObject(int);
	int removeObject(int osj);
	void fliplay(const char *filefli, int vel);
	void fadeFromBlack(int fadeSpeed);
	char adjustToVGA(char value);
	void color_abc(int cl);
	void centerText(const char *,int,int);
	void playSound(int soundNum);
	bool animate(const char *animation, int FPS);
	void fadeToBlack(int fadeSpeed);
	void pause(int);
	void placeIgor();
	void placeBJ();
	void placeDrascula();

	void talkInit(const char *filename);
	bool isTalkFinished(int* length);
	void talk_igor(int, int);
	void talk_drascula(int index, int talkerType = 0);
	void talk_solo(const char *, const char *);
	void talk_bartender(int, int talkerType = 0);
	void talk_pen(const char *, const char *, int);
	void talk_bj_bed(int);
	void talk_htel(int);
	void talk_bj(int);
	void talk_baul(int);
	void talk(int);
	void talk(const char *, const char *);
	void talk_sinc(const char *, const char *, const char *);
	void talk_drunk(int);
	void talk_pianist(int);
	void talk_wolf(int);
	void talk_mus(int);
	void talk_dr_grande(int);
	void talk_vb(int);
	void talk_vbpuerta(int);
	void talk_blind(int);
	void talk_hacker(const char *, const char *);

	void hiccup(int);
	void finishSound();
	void stopSound();
	void closeDoor(int nflag, int doorNum);
	void playMusic(int p);
	void stopMusic();
	int musicStatus();
	void updateRoom();
	bool loadGame(const char *);
	void updateDoor(int);
	void color_hare();
	void funde_hare(int oscuridad);
	void paleta_hare_claro();
	void paleta_hare_oscuro();
	void hare_claro();
	void updateVisible();
	void startWalking();
	void updateRefresh();
	void updateRefresh_pre();
	void pon_hare();
	void showMenu();
	void clearMenu();
	void removeObject();
	bool exitRoom(int);
	bool pickupObject();
	bool checkFlag(int);
	void setCursorTable();
	void enterName();
	void para_grabar(char[]);
	bool soundIsActive();
	void openSSN(const char *Name, int Pause);
	void WaitFrameSSN();
	void MixVideo(byte *OldScreen, byte *NewScreen);
	void Des_RLE(byte *BufferRLE, byte *MiVideoRLE);
	void Des_OFF(byte *BufferOFF, byte *MiVideoOFF, int Lenght);
	void set_dacSSN(byte *dacSSN);
	byte *TryInMem();
	void EndSSN();
	int playFrameSSN();

	byte *AuxBuffOrg;
	byte *AuxBuffLast;
	byte *AuxBuffDes;

	byte *pointer;
	int UsingMem;
	byte CHUNK;
	byte CMP, dacSSN[768];
	byte *MiVideoSSN;
	byte *mSession;
	int FrameSSN;
	int globalSpeed;
	uint32 LastFrame;

	int frame_pen;
	int flag_tv;

	byte *loadPCX(byte *NamePcc);
	void set_dac(byte *dac);
	void WaitForNext(int FPS);
	int getTime();
	void reduce_hare_chico(int, int, int, int, int, int, int, byte *, byte *);
	void quadrant_1();
	void quadrant_2();
	void quadrant_3();
	void quadrant_4();
	void saveGame(char[]);
	void increaseFrameNum();
	int whichObject();
	bool checkMenuFlags();
	bool roomParse(RoomTalkAction*, int);
	void converse(const char *);
	void print_abc_opc(const char *, int, int, int);
	void response(int);
	void room_pendulum(int);
	void update_pendulum();
	void activatePendulum();

	void MusicFadeout();
	void playFile(const char *fname);

	char *getLine(char *buf, int len);
	void getIntFromLine(char *buf, int len, int* result);
	void getStringFromLine(char *buf, int len, char* result);

	void grr();
	void updateAnim(int y, int destX, int destY, int width, int height, int count, byte* src, int delayVal = 3);
	void updateAnim2(int y, int px, int py, int width, int height, int count, byte* src);

	void room_0();
	void room_1(int);
	void room_2(int);
	void room_3(int);
	void room_4(int);
	void room_5(int);
	void room_6(int);
	void room_7(int);
	void room_8(int);
	void room_9(int);
	void room_12(int);
	bool room_13(int fl);
	void room_14(int);
	void room_15(int);
	void room_16(int);
	void room_17(int);
	void room_18(int);
	void room_19(int);
	bool room_21(int);
	void room_22(int);
	void room_23(int);
	void room_24(int);
	void room_26(int);
	void room_27(int);
	void room_29(int);
	void room_30(int);
	void room_31(int);
	void room_34(int);
	void room_35(int);
	void room_44(int);
	void room_49(int);
	void room_53(int);
	void room_54(int);
	void room_55(int);
	bool room_56(int);
	void room_58(int);
	void room_59(int);
	bool room_60(int);
	void room_61(int);
	void room_62(int);
	void room_63(int);

	void animation_1_1();
	void animation_2_1();
	void animation_3_1();
	void animation_4_1();
	//
	void animation_1_2();
	void animation_2_2();
	void animation_3_2();
	void animation_4_2();
	void animation_5_2();
	void animation_6_2();
	void animation_7_2();
	void animation_8_2();
	void animation_9_2();
	void animation_10_2();
	void animation_11_2();
	void animation_12_2();
	void animation_13_2();
	void animation_14_2();
	void animation_15_2();
	void animation_16_2();
	void animation_17_2();
	void animation_18_2();
	void animation_19_2();
	void animation_20_2();
	void animation_21_2();
	void animation_22_2();
	void animation_23_2();
	void animation_23_joined();
	void animation_23_joined2();
	void animation_24_2();
	void animation_25_2();
	void animation_26_2();
	void animation_27_2();
	void animation_28_2();
	void animation_29_2();
	void animation_30_2();
	void animation_31_2();
	void animation_32_2();
	void animation_33_2();
	void animation_34_2();
	void animation_35_2();
	void animation_36_2();
	//
	void animation_1_3();
	void animation_2_3();
	void animation_3_3();
	void animation_4_3();
	void animation_5_3();
	void animation_6_3();
	void animation_rayo();
	//
	void animation_1_4();
	void animation_2_4();
	void animation_3_4();
	void animation_4_4();
	void animation_5_4();
	void animation_6_4();
	void animation_7_4();
	void animation_8_4();
	void animation_9_4();
	//
	void animation_1_5();
	void animation_2_5();
	void animation_3_5();
	void animation_4_5();
	void animation_5_5();
	void animation_6_5();
	void animation_7_5();
	void animation_8_5();
	void animation_9_5();
	void animation_10_5();
	void animation_11_5();
	void animation_12_5();
	void animation_13_5();
	void animation_14_5();
	void animation_15_5();
	void animation_16_5();
	void animation_17_5();
	//
	void animation_1_6();
	void animation_2_6();
	void animation_3_6();
	void animation_4_6();
	void animation_5_6();
	void animation_6_6();
	void animation_7_6();
	void animation_9_6();
	void animation_10_6();
	void animation_11_6();
	void animation_12_6();
	void animation_13_6();
	void animation_14_6();
	void animation_15_6();
	void animation_18_6();
	void animation_19_6();

	void update_1_pre();
	void update_2();
	void update_3();
	void update_3_pre();
	void update_4();
	void update_5();
	void update_5_pre();
	void update_6_pre();
	void update_7_pre();
	void update_9_pre();
	void update_12_pre();
	void update_14_pre();
	void update_13();
	void update_15();
	void update_16_pre();
	void update_17_pre();
	void update_17();
	void update_18_pre();
	void update_18();
	void update_20();
	void update_21_pre();
	void update_22_pre();
	void update_23_pre();
	void update_24_pre();
	void update_26_pre();
	void update_26();
	void update_27();
	void update_27_pre();
	void update_29();
	void update_29_pre();
	void update_30_pre();
	void update_31_pre();
	void update_34_pre();
	void update_35_pre();
	void update_31();
	void update_34();
	void update_35();
	void update_53_pre();
	void update_54_pre();
	void update_49_pre();
	void update_56_pre();
	void update_50();
	void update_57();
	void update_58();
	void update_58_pre();
	void update_59_pre();
	void update_60_pre();
	void update_60();
	void update_61();
	void update_62();
	void update_62_pre();
	void update_63();

private:
	int _lang;
};

extern const char *_text[][501];
extern const char *_textd[][84];
extern const char *_textb[][15];
extern const char *_textbj[][29];
extern const char *_texte[][24];
extern const char *_texti[][33];
extern const char *_textl[][32];
extern const char *_textp[][20];
extern const char *_textt[][25];
extern const char *_textvb[][63];
extern const char *_textsys[][4];
extern const char *_texthis[][5];
extern const char *_textverbs[][6];
extern const char *_textmisc[][2];
extern const char *_textd1[][11];

extern const ItemLocation itemLocations[];
extern int frame_x[20];
extern const int x_pol[44], y_pol[44];
extern const int verbBarX[];
extern const int x1d_menu[], y1d_menu[];

extern const CharInfo charMap[];

} // End of namespace Drascula

#endif /* DRASCULA_H */
