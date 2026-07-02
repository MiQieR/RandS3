//
// Slot.h
//
#pragma once

#include <M5Unified.h>

#define MAX_SLOT_COUNT 5 // Max columns that fit on 240px width

enum SlotState { SLOT_INIT, SLOT_START, SLOT_STOP, SLOT_DRIFT };

class Slot {
public:
	Slot() {}
	void init(int unit, int total_columns, int index);
	void draw(int offsetX = 0, int offsetY = 0);
	void flush(uint16_t bgColor = 0xF800, int offsetX = 0, int offsetY = 0);
	void start(int acc = 12, int maxVel = 720);
	void stop(int acc = -20, int minVel = 50);
	void setTargetSymbol(int symbol) { targetSymbol = symbol; }
	int getSymbol() { return (index == -1) ? -1 : symbolIndices[index]; }
	bool update();

	static void initShadow(int shadowHeight = 16);
	static void setReel(const int *symbolIndices, int reelLength);

private:
	int index;
	int posX;
	int height;
	float degree;
	int vel; // deg/s
	int maxVel;
	int minVel;
	int acc; // deg/s^2
	SlotState state;
	unsigned long tick;
	int targetSymbol = -1;

	static const int *symbolIndices;
	static int reelLength;
	static int reelHeight;
	static int shadowHeight;
	static uint8_t *shadowBrigtness;
	static uint16_t *buffer;

	static uint16_t darker(uint16_t rgb, uint8_t brightness);
	static uint8_t calcBrightness(float step);
	static float sigmoid(float t);
};
