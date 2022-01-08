
#pragma once
#include "stdafx.h"




bool TestBool = false;
int selectedPlayer;

char* CharKeyboard(char* windowName = "", int maxInput = 21, char* defaultText = "") {
	GAMEPLAY::DISPLAY_ONSCREEN_KEYBOARD(0, "", "", defaultText, "", "", "", maxInput);
	while (GAMEPLAY::UPDATE_ONSCREEN_KEYBOARD() == 0) WAIT(0);
	if (!GAMEPLAY::GET_ONSCREEN_KEYBOARD_RESULT()) return "";
	return GAMEPLAY::GET_ONSCREEN_KEYBOARD_RESULT();
}
int NumberKeyboard() {
	GAMEPLAY::DISPLAY_ONSCREEN_KEYBOARD(1, "", "", "", "", "", "", 10);
	while (GAMEPLAY::UPDATE_ONSCREEN_KEYBOARD() == 0) WAIT(0);
	if (!GAMEPLAY::GET_ONSCREEN_KEYBOARD_RESULT()) return 0;
	return atof(GAMEPLAY::GET_ONSCREEN_KEYBOARD_RESULT());
}
void notifyleft(char* msg)
{
	UI::BEGIN_TEXT_COMMAND_THEFEED_POST("STRING");
	UI::ADD_TEXT_COMPONENT_SUBSTRING_PLAYER_NAME(msg);
	UI::END_TEXT_COMMAND_THEFEED_POST_TICKER(2000, 1);
}

Vector3 TPCoords;
void TPto(Vector3 Coords)
{
	int Handle = PLAYER::PLAYER_PED_ID();
	if (PED::IS_PED_IN_ANY_VEHICLE(Handle, 0))
	{
		ENTITY::SET_ENTITY_COORDS(PED::GET_VEHICLE_PED_IS_IN(Handle, false), Coords.x, Coords.y, Coords.z, 0, 0, 0, 1);
	}
	else
		ENTITY::SET_ENTITY_COORDS(Handle, Coords.x, Coords.y, Coords.z, 0, 0, 0, 1);
}
void Features()
{
	if (TestBool)
	{
		//Code to Execute
	}
	else
	{
		//Code to Execute
	}
}


void main() {

	while (true) {
		Menu::Checks::Controlls();
		Features();
		notifyMap("~f~Nano 42 Loaded!");
		switch (Menu::Settings::currentMenu) {

		case mainmenu:
		{
			Menu::Title("~b~D~r~o~g~w~bold~n");
			Menu::MenuOption("Sub Menu ~b~>", sub);
			Menu::MenuOption("Player List ~b~>", plist);
			Menu::MenuOption("Settings ~b~>", settings);
		}
		break;
#pragma region Self Menu
		case plist:
		{
			Menu::Title("Player List");
			for (int i; i < 32; i++) {
				if (NETWORK::NETWORK_IS_PLAYER_CONNECTED(i)) {
					Menu::PlayerOption(i);
				}
			}
		}
			break;
		case pmenu:
		{
			Menu::Title(PLAYER::GET_PLAYER_NAME(selectedPlayer));
			Menu::Break("It Works!");
		}
			break;
		case sub :
		{
			Menu::Title("Sub Menu");
			Menu::Toggle("Test Bool", TestBool);
			//To use functions do _>
			if (Menu::Option("Toggle Function"))
			{
				//Code to execute
			}
			//SubMenu
		}
		break;
#pragma endregion
#pragma region Settings Menu
		case settings:
		{
			Menu::Title("Settings");
			Menu::MenuOption("Colors ~b~>", settingstheme);
			if (Menu::Int("Scroll Delay", Menu::Settings::keyPressDelay2, 1, 200))
			{
				if (IsKeyPressed(Menu::Settings::selectKey) || CONTROLS::IS_DISABLED_CONTROL_PRESSED(0, ControlFrontendAccept)) {
					Menu::Settings::keyPressDelay2 = NumberKeyboard();
				}
			}
			if (Menu::Int("Int Delay", Menu::Settings::keyPressDelay3, 1, 200))
			{
				if (IsKeyPressed(Menu::Settings::selectKey) || CONTROLS::IS_DISABLED_CONTROL_PRESSED(0, ControlFrontendAccept)) {
					Menu::Settings::keyPressDelay3 = NumberKeyboard();
				}
			}
			Menu::Option("~HUD_COLOUR_GOLD~Social Club Account:");
			Menu::Option(PLAYER::GET_PLAYER_NAME(PLAYER::PLAYER_ID()));
			Menu::MenuOption("~r~KILL GAME", exitgta);
		}
		break;
		case Credits:
		{
			Menu::Title("Credits");
			Menu::Option("Base : ~r~maxiZzModz");
			Menu::Option("Modified By : ~f~CptShad");
			Menu::Option("Updated By : ~HUD_COLOUR_GOLD~AdryaNN53");
			Menu::Option("Thanks to:");
			Menu::Option("~b~LHDModz");
			Menu::Option("~p~Mike Rohsoft");
			Menu::Option("Taran VG");
			Menu::Option("~r~Name");
			Menu::Option("V. 0.0");
		}
		break;
		case exitgta:
		{
			Menu::Title("Exit GTA V");
			if (Menu::Option("Yes")) exit(0);
		}
		break;
		case settingstheme:
		{
			Menu::Title("Colors");
			Menu::MenuOption("Theme Loader ~b~>", themeloader);
			Menu::MenuOption("Title Background ~b~>", settingstitlerect);
			Menu::MenuOption("Selection Box ~b~>", settingsscroller);
			Menu::MenuOption("Option Text ~b~>", settingsoptiontext);
			if (Menu::Option("MenuX plus")) {
				if (Menu::Settings::menuX < 0.81f) Menu::Settings::menuX += 0.01f;
			}
			if (Menu::Option("MenuX minus")) {
				if (Menu::Settings::menuX > 0.17f) Menu::Settings::menuX -= 0.01f;
			}
		}
		break;
		case themeloader:
		{
			Menu::Title("Theme Colors");
			if (Menu::Option("Red Theme")) {
				Menu::Settings::titleRect = { 100, 0, 0, 255 };
				Menu::Settings::scroller = { 100, 0, 0, 255 };
			}
			if (Menu::Option("Blue Theme")) {
				Menu::Settings::titleRect = { 0, 0, 200, 255 };
				Menu::Settings::scroller = { 0, 0, 200, 255 };
			}
			if (Menu::Option("Green Theme")) {
				Menu::Settings::titleRect = { 0, 180, 0, 255 };
				Menu::Settings::scroller = { 0, 0, 180, 255 };
			}
			if (Menu::Option("Load Default Theme")) {
				Menu::Settings::titleText = { 255, 255, 255, 255, 7 };
				Menu::Settings::titleRect = { 100, 0, 0, 255 };
				Menu::Settings::scroller = { 100, 0, 0, 255 };
				Menu::Settings::optionText = { 255, 255, 255, 255, 0 };
				Menu::Settings::optionRect = { 0, 0, 0, 110 };
			}
		}
		break;
		case settingstitlerect:
		{
			Menu::Title("Title Rect");
			if (Menu::Int("Red", Menu::Settings::titleRect.r, 0, 255))
			{
				if (IsKeyPressed(Menu::Settings::selectKey) || CONTROLS::IS_DISABLED_CONTROL_PRESSED(0, ControlFrontendAccept)) {
					Menu::Settings::titleRect.r = NumberKeyboard();
				}
			}
			if (Menu::Int("Green", Menu::Settings::titleRect.g, 0, 255))
			{
				if (IsKeyPressed(Menu::Settings::selectKey) || CONTROLS::IS_DISABLED_CONTROL_PRESSED(0, ControlFrontendAccept)) {
					Menu::Settings::titleRect.g = NumberKeyboard();
				}
			}
			if (Menu::Int("Blue", Menu::Settings::titleRect.b, 0, 255))
			{
				if (IsKeyPressed(Menu::Settings::selectKey) || CONTROLS::IS_DISABLED_CONTROL_PRESSED(0, ControlFrontendAccept)) {
					Menu::Settings::titleRect.b = NumberKeyboard();
				}
			}
			if (Menu::Int("Opacity", Menu::Settings::titleRect.a, 0, 255))
			{
				if (IsKeyPressed(Menu::Settings::selectKey) || CONTROLS::IS_DISABLED_CONTROL_PRESSED(0, ControlFrontendAccept)) {
					Menu::Settings::titleRect.a = NumberKeyboard();
				}
			}
		}
		break;
		case settingsoptiontext:
		{
			Menu::Title("Option Text");
			if (Menu::Int("Red", Menu::Settings::optionText.r, 0, 255))
			{
				if (IsKeyPressed(Menu::Settings::selectKey) || CONTROLS::IS_DISABLED_CONTROL_PRESSED(0, ControlFrontendAccept)) {
					Menu::Settings::optionText.r = NumberKeyboard();
				}
			}
			if (Menu::Int("Green", Menu::Settings::optionText.g, 0, 255))
			{
				if (IsKeyPressed(Menu::Settings::selectKey) || CONTROLS::IS_DISABLED_CONTROL_PRESSED(0, ControlFrontendAccept)) {
					Menu::Settings::optionText.g = NumberKeyboard();
				}
			}
			if (Menu::Int("Blue", Menu::Settings::optionText.b, 0, 255))
			{
				if (IsKeyPressed(Menu::Settings::selectKey) || CONTROLS::IS_DISABLED_CONTROL_PRESSED(0, ControlFrontendAccept)) {
					Menu::Settings::optionText.b = NumberKeyboard();
				}
			}
			if (Menu::Int("Opacity", Menu::Settings::optionText.a, 0, 255))
			{
				if (IsKeyPressed(Menu::Settings::selectKey) || CONTROLS::IS_DISABLED_CONTROL_PRESSED(0, ControlFrontendAccept)) {
					Menu::Settings::optionText.a = NumberKeyboard();
				}
			}
			Menu::MenuOption("Font ~b~>", font);
		}
		break;
		case font:
		{
			Menu::Title("Font");
			if (Menu::Option("Chalet London")) { Menu::Settings::optionText.f = 0; }
			if (Menu::Option("House Script")) { Menu::Settings::optionText.f = 1; }
			if (Menu::Option("Monospace")) { Menu::Settings::optionText.f = 2; }
			if (Menu::Option("Wing Dings")) { Menu::Settings::optionText.f = 3; }
			if (Menu::Option("Chalet Comprime Cologne")) { Menu::Settings::optionText.f = 4; }
			if (Menu::Option("Pricedown")) { Menu::Settings::optionText.f = 7; }
		}
		break;
		case settingsscroller:
		{
			Menu::Title("Scroller");
			if (Menu::Int("Red", Menu::Settings::scroller.r, 0, 255))
			{
				if (IsKeyPressed(Menu::Settings::selectKey) || CONTROLS::IS_DISABLED_CONTROL_PRESSED(0, ControlFrontendAccept)) {
					Menu::Settings::scroller.r = NumberKeyboard();
				}
			}
			if (Menu::Int("Green", Menu::Settings::scroller.g, 0, 255))
			{
				if (IsKeyPressed(Menu::Settings::selectKey) || CONTROLS::IS_DISABLED_CONTROL_PRESSED(0, ControlFrontendAccept)) {
					Menu::Settings::scroller.g = NumberKeyboard();
				}
			}
			if (Menu::Int("Blue", Menu::Settings::scroller.b, 0, 255))
			{
				if (IsKeyPressed(Menu::Settings::selectKey) || CONTROLS::IS_DISABLED_CONTROL_PRESSED(0, ControlFrontendAccept)) {
					Menu::Settings::scroller.b = NumberKeyboard();
				}
			}
			if (Menu::Int("Opacity", Menu::Settings::scroller.a, 0, 255))
			{
				if (IsKeyPressed(Menu::Settings::selectKey) || CONTROLS::IS_DISABLED_CONTROL_PRESSED(0, ControlFrontendAccept)) {
					Menu::Settings::scroller.a = NumberKeyboard();
				}
			}
		}
		break;
#pragma endregion
		}
		Menu::End();
		WAIT(0);
	}
}

void ScriptMain() {
	srand(GetTickCount());

	main();
}