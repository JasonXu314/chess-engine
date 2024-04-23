#ifndef CHESS_H
#define CHESS_H

#include <iostream>
#include <string>
#include <vector>

#include "constants.h"

struct Position {
	Files file;
	uint rank;
};

bool operator==(const Position& a, const Position& b);

std::string to_string(const Position& pos);

struct Move {
	Position from;
	Position to;
};

class Piece {
public:
	Piece(char symbol, Position position);

	const Position& position() const;

	PieceTypes type() const;

	Players player() const;

	char symbol() const;

	friend class Game;
	friend std::ostream& operator<<(std::ostream& out, const Piece& piece);

private:
	Position _position;
	PieceTypes _type;
	uint _value;
	Players _player;
	char _symbol;
	bool _moved;
};

std::ostream& operator<<(std::ostream& out, const Piece& piece);

class Game {
public:
	Game();
	Game(const Game& other);
	Game(const std::string& fen);

	// returns true if pawn reached promotion (also sets shouldPromote private variable)
	bool move(const Move& move);

	// Evaluates the move in a new game (for hypothetical futures); note the const
	Game branch(const Move& move) const;
	Game branchPromote(const Position& pos, PieceTypes to) const;

	std::vector<Move> getAvailableMoves() const;

	uint materiel(Players player) const;

	void promote(const Position& pos, PieceTypes to);

	bool isChecked() const;
	bool isChecked(Players player) const;

	std::string dumpFEN() const;

	Piece getPiece(const Position& pos) const;

	bool hasPiece(const Position& pos) const;

	Players turn() const;

	bool shouldPromote() const;

	int turns() const { return _turns; }

	int halfTurnsSinceCapture() const { return _halfTurnsSinceCapture; }

	Game& operator=(const Game& other);

private:
	std::vector<Piece> _white;
	std::vector<Piece> _black;
	Players _turn;
	bool _firstMove;
	Move _prevMove;
	bool _prevMoveEnPassant;
	bool _shouldPromote;
	int _turns;
	int _halfTurnsSinceCapture;

	Piece& _getPieceRef(const Position& pos);

	Game _uncheckedBranch(const Move& move) const;

	void _validatePawnMove(const Move& move) const;
	void _validateKnightMove(const Move& move) const;
	void _validateBishopMove(const Move& move) const;
	void _validateRookMove(const Move& move) const;
	void _validateKingMove(const Move& move) const;
};

#endif