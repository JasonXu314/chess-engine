#include <curses.h>

#include <iostream>

#include "chess.h"

using namespace std;

void printBoard(const Game& game, bool squareSelected, const Position& selectedSquare);

int main() {
	int ch = '\0';
	MEVENT event;

	initscr();
	if (!has_colors()) {
		endwin();
		cout << "Your terminal does not support colors." << endl;
		return 1;
	}
	start_color();

	noecho();
	cbreak();
	keypad(stdscr, true);
	mousemask(BUTTON1_CLICKED | BUTTON2_CLICKED | BUTTON3_CLICKED, nullptr);
	init_pair(1, COLOR_WHITE, COLOR_GREEN);
	init_pair(2, COLOR_BLACK, COLOR_RED);

	Game game;
	bool squareSelected = false;
	Position selectedSquare;
	string error;

	do {
		switch (ch) {
			case KEY_MOUSE:
				if (getmouse(&event) == OK) {
					if (event.bstate & BUTTON1_CLICKED) {
						if (event.y < 8 && event.x / 2 < 8) {
							if (squareSelected) {
								try {
									game.move({.from = selectedSquare, .to = {.file = (Files)(event.x / 2), .rank = (uint)(8 - event.y)}});
									squareSelected = false;
									error = "";
								} catch (const runtime_error& e) {
									error = e.what();
								} catch (...) {
									error = "Unknown error";
								}
							} else {
								squareSelected = true;
								selectedSquare = {.file = (Files)(event.x / 2), .rank = (uint)(8 - event.y)};
								error = "";
							}
						}
					} else if (event.bstate & BUTTON2_CLICKED || event.bstate & BUTTON3_CLICKED) {
						squareSelected = false;
						error = "";
					}
				}
				break;
			case '\0':
				break;
			default:
				goto skip;
		}

		clear();
		printBoard(game, squareSelected, selectedSquare);
		if (squareSelected) {
			mvprintw(8, 0, to_string(selectedSquare).c_str());
		}
		mvprintw(9, 0, game.turn() == Players::WHITE ? "White to move" : "Black to move");
		if (error != "") {
			attron(COLOR_PAIR(2));
			mvprintw(10, 0, error.c_str());
			attroff(COLOR_PAIR(2));
		}
		mvprintw(15, 0, "White is uppercase, black is lowercase");
		mvprintw(16, 0, "Click to select a square, click again to move");
		mvprintw(17, 0, "Middle/right click to deselect");
		refresh();
	skip:
		void(0);
	} while ((ch = getch()) != 'q');
	endwin();

	return 0;
}

void printBoard(const Game& game, bool squareSelected, const Position& selectedSquare) {
	bool black = true;

	for (Files file : FILES) {
		for (uint rank = 1; rank <= 8; rank++) {
			Position square = {.file = file, .rank = rank};

			if (game.hasPiece(square)) {
				Piece piece = game.getPiece(square);

				if (squareSelected && square == selectedSquare)
					attron(COLOR_PAIR(1));
				else if (!black)
					attron(A_REVERSE);
				mvaddch(8 - rank, file * 2, ' ');
				addch(piece.symbol());
				if (squareSelected && square == selectedSquare)
					attroff(COLOR_PAIR(1));
				else if (!black)
					attroff(A_REVERSE);
			} else {
				if (squareSelected && square == selectedSquare)
					attron(COLOR_PAIR(1));
				else if (!black)
					attron(A_REVERSE);
				mvaddch(8 - rank, file * 2, ' ');
				addch(' ');
				if (squareSelected && square == selectedSquare)
					attroff(COLOR_PAIR(1));
				else if (!black)
					attroff(A_REVERSE);
			}

			black = !black;
		}

		black = !black;
	}
}