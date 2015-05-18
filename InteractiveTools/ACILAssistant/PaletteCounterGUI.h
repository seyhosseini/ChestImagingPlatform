// generated by Fast Light User Interface Designer (fluid) version 1.0300

#ifndef PaletteCounterGUI_h
#define PaletteCounterGUI_h
#include <FL/Fl.H>
#include <string>
#include <FL/Fl_Text_Buffer.H>
#include <sstream>
#include <stdlib.h>
#include <stdio.h>
using namespace std;
#include <FL/Fl_Double_Window.H>
#include <FL/Fl_Text_Display.H>

class PaletteCounterGUI {
public:
  PaletteCounterGUI();
  Fl_Double_Window *paletteCounterWindow;
  Fl_Text_Display *chestRegionField;
  Fl_Text_Display *chestTypeField;
  Fl_Text_Display *countField;
private:
  std::string m_ChestRegion; 
  std::string m_ChestType; 
  Fl_Text_Buffer* m_ChestTypeBuffer; 
  Fl_Text_Buffer* m_ChestRegionBuffer; 
  Fl_Text_Buffer* m_CountBuffer; 
public:
  void SetChestRegion(std::string cipRegion);
  void SetChestType(std::string cipType);
  void SetCount(unsigned int count);
};
#endif
