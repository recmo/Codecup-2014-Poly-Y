#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include <inttypes.h>
#include <cassert>
#include <cstdlib>
#include <sstream>
#include <cmath>
#include <sys/stat.h>
using namespace std;
typedef unsigned int uint;
typedef unsigned char uint8;
typedef uint64_t uint64;
typedef signed int sint;

inline uint popcount(uint64 n)
{
	return __builtin_popcountll(n);
}

inline uint trailingZeros(uint64 n)
{
	return __builtin_ctzll(n);
}

inline double randomReal()
{
	return static_cast<double>(rand()) / static_cast<double>(RAND_MAX);
}

class BoardPoint;
class BoardMask;
class Board;
class Move;
class TreeNode;

class BoardPoint {
public:
	BoardPoint(): _position(0) { }
	BoardPoint(const string& str);
	BoardPoint(uint position): _position(position) { }
	~BoardPoint() { }
	
	bool operator==(const BoardPoint& other) const { return _position == other._position; }
	bool isValid() const { return _position >= 1 && _position <= 106; }
	uint position() const { return _position; }
	BoardMask mask() const;
	BoardMask neighbors() const;
	BoardPoint& position(uint value) { _position = value; return *this; }
	
protected:
	static const BoardMask _neighbors[106];
	uint _position; /// [1...106] inclusive
};

std::ostream& operator<<(std::ostream& out, const BoardPoint& point)
{
	out << point.position();
	return out;
}

std::istream& operator>>(std::istream& in, BoardPoint& point)
{
	uint position;
	in >> position;
	point = BoardPoint(position);
	return in;
}

BoardPoint::BoardPoint(const string& str)
: _position(0)
{
	stringstream stream(str);
	stream >> *this;
}


class Move: public BoardPoint {
public:
	static Move Swap;
	
	Move(): BoardPoint(0) { }
	Move(const BoardPoint& point): BoardPoint(point) { }
	Move(const string& str);
	Move(uint position): BoardPoint(position) { }
	~Move() { }
	
	bool isValid() const { return BoardPoint::isValid() || (*this == Swap); }
	
protected:
	static const BoardMask _neighbors[106];
	uint _position;
};

Move Move::Swap(107);

Move::Move(const string& str)
: BoardPoint(0)
{
	stringstream stream(str);
	stream >> *this;
}

std::ostream& operator<<(std::ostream& out, const Move& move)
{
	if(move == Move::Swap)
		out << "-1";
	else
		out << move.position();
	return out;
}

std::istream& operator>>(std::istream& in, Move& move)
{
	int position;
	in >> position;
	move = (position == -1) ? Move::Swap : Move(position);
	return in;
}

class BoardMask {
public:
	class Itterator;
	const static BoardMask fullBoard;
	const static BoardMask borders[5];
	
	BoardMask(): _a(0), _b(0) { }
	BoardMask(const BoardMask& other): _a(other._a), _b(other._b) { }
	BoardMask(BoardPoint point);
	~BoardMask() { }
	operator bool() const { return !isEmpty(); }
	BoardMask& operator=(const BoardMask& other) { _a = other._a; _b = other._b; return *this; }
	bool operator==(const BoardMask& other) const { return _a == other._a && _b == other._b; }
	bool operator!=(const BoardMask& other) const { return !operator==(other); }
	BoardMask& operator&=(const BoardMask& other) { return operator=(operator&(other)); }
	BoardMask& operator|=(const BoardMask& other) { return operator=(operator|(other)); }
	BoardMask& operator-=(const BoardMask& other) { return operator=(operator-(other)); }
	BoardMask operator&(const BoardMask& other) const { return BoardMask(_a & other._a, _b & other._b); }
	BoardMask operator|(const BoardMask& other) const { return BoardMask(_a | other._a, _b | other._b); }
	BoardMask operator-(const BoardMask& other) const { return *this & (~other); }
	BoardMask operator~() const { return BoardMask(~_a, ~_b) & fullBoard; }
	BoardMask expanded() const;
	BoardMask connected(const BoardMask& seed) const;
	vector<BoardMask> groups() const;
	uint controlledCorners() const;
	BoardMask& invert() { return operator=(operator~()); }
	BoardMask& expand() { return operator=(expanded()); }
	BoardMask& clear() { return operator=(BoardMask()); }
	uint popcount() const { return ::popcount(_a) + ::popcount(_b); }
	bool isSet(BoardPoint point) const { return !(*this & BoardMask(point)).isEmpty(); }
	BoardMask& set(BoardPoint point) { return operator=(*this | BoardMask(point)); }
	BoardMask& clear(BoardPoint point) { return operator=(*this & ~BoardMask(point)); }
	bool isEmpty() const { return operator==(BoardMask()); }
	bool isValid() const { return operator==(*this & fullBoard); }
	BoardPoint firstPoint() const;
	BoardPoint randomPoint() const;
	Itterator itterator() const;
	
	uint64 a() const { return _a; }
	uint64 b() const { return _b; }
	
protected:
	BoardMask(uint64 a, uint64 b) :_a(a), _b(b) { }
	
protected:
	uint64 _a;
	uint64 _b;
};

class BoardMask::Itterator {
public:
	Itterator(const BoardMask& mask): _mask(mask), _point(_mask.firstPoint()) { }
	~Itterator() { }
	operator bool() const { return _mask._a | _mask._b; }
	Itterator& operator++() { _mask.clear(_point); _point = _mask.firstPoint(); return *this; }
	Itterator operator++(int) { Itterator tmp(*this); operator++(); return tmp; }
	const BoardPoint& operator*() const { return _point; }
	const BoardPoint* operator->() const { return &_point; }
	
private:
	BoardMask _mask;
	BoardPoint _point;
};

std::ostream& operator<<(std::ostream& out, const BoardMask& mask)
{
	for(uint i = 1; i <= 106; ++i)
		out << (mask.isSet(i) ? "#" : ".");
	return out;
}

const BoardMask BoardPoint::_neighbors[106] = {
	BoardMask().set(2).set(3).set(4), // 1
	BoardMask().set(1).set(3).set(5).set(6), // 2
	BoardMask().set(1).set(2).set(6).set(7).set(8).set(4), // 3
	BoardMask().set(1).set(3).set(8).set(9), // 4
	BoardMask().set(2).set(6).set(11).set(10), // 5
	BoardMask().set(2).set(5).set(11).set(12).set(7).set(3), // 6
	BoardMask().set(3).set(6).set(12).set(13).set(14).set(8), // 7
	BoardMask().set(4).set(3).set(7).set(14).set(15).set(9), // 8
	BoardMask().set(4).set(8).set(15).set(16), // 9
	BoardMask().set(5).set(11).set(18).set(17), // 10
	BoardMask().set(5).set(10).set(18).set(19).set(12).set(6), // 11
	BoardMask().set(6).set(11).set(19).set(20).set(13).set(7), // 12
	BoardMask().set(7).set(12).set(20).set(21).set(22).set(14), // 13
	BoardMask().set(8).set(7).set(13).set(22).set(23).set(15), // 14
	BoardMask().set(9).set(8).set(14).set(23).set(24).set(16), // 15
	BoardMask().set(9).set(15).set(24).set(25), // 16
	BoardMask().set(10).set(18).set(26).set(27), // 17
	BoardMask().set(10).set(17).set(27).set(28).set(19).set(11), // 18
	BoardMask().set(11).set(18).set(28).set(29).set(20).set(12), // 19
	BoardMask().set(12).set(19).set(29).set(30).set(21).set(13), // 20
	BoardMask().set(13).set(20).set(30).set(31).set(32).set(22), // 21
	BoardMask().set(14).set(13).set(21).set(32).set(33).set(23), // 22
	BoardMask().set(15).set(14).set(22).set(33).set(34).set(24), // 23
	BoardMask().set(16).set(15).set(23).set(34).set(35).set(25), // 24
	BoardMask().set(16).set(24).set(35).set(36), // 25
	BoardMask().set(17).set(27).set(37).set(38), // 26
	BoardMask().set(17).set(26).set(38).set(39).set(28).set(18), // 27
	BoardMask().set(18).set(27).set(39).set(40).set(29).set(19), // 28
	BoardMask().set(19).set(28).set(40).set(41).set(30).set(20), // 29
	BoardMask().set(20).set(29).set(41).set(42).set(31).set(21), // 30
	BoardMask().set(21).set(30).set(42).set(43).set(44).set(32), // 31
	BoardMask().set(22).set(21).set(31).set(44).set(45).set(33), // 32
	BoardMask().set(23).set(22).set(32).set(45).set(46).set(34), // 33
	BoardMask().set(24).set(23).set(33).set(46).set(47).set(35), // 34
	BoardMask().set(25).set(24).set(34).set(47).set(48).set(36), // 35
	BoardMask().set(25).set(35).set(48).set(49), // 36
	BoardMask().set(26).set(38).set(50), // 37
	BoardMask().set(26).set(37).set(50).set(51).set(39).set(27), // 38
	BoardMask().set(27).set(38).set(51).set(52).set(40).set(28), // 39
	BoardMask().set(28).set(39).set(52).set(53).set(41).set(29), // 40
	BoardMask().set(29).set(40).set(53).set(54).set(42).set(30), // 41
	BoardMask().set(30).set(41).set(54).set(55).set(43).set(31), // 42
	BoardMask().set(31).set(42).set(55).set(56).set(44), // 43
	BoardMask().set(31).set(43).set(56).set(57).set(45).set(32), // 44
	BoardMask().set(32).set(44).set(57).set(58).set(46).set(33), // 45
	BoardMask().set(33).set(45).set(58).set(59).set(47).set(34), // 46
	BoardMask().set(34).set(46).set(59).set(60).set(48).set(35), // 47
	BoardMask().set(35).set(47).set(60).set(61).set(49).set(36), // 48
	BoardMask().set(36).set(48).set(61), // 49
	BoardMask().set(37).set(38).set(51).set(62), // 50
	BoardMask().set(38).set(50).set(62).set(63).set(52).set(39), // 51
	BoardMask().set(39).set(51).set(63).set(64).set(53).set(40), // 52
	BoardMask().set(40).set(52).set(64).set(65).set(54).set(41), // 53
	BoardMask().set(41).set(53).set(65).set(66).set(55).set(42), // 54
	BoardMask().set(42).set(54).set(66).set(67).set(56).set(43), // 55
	BoardMask().set(43).set(55).set(67).set(68).set(57).set(44), // 56
	BoardMask().set(44).set(56).set(68).set(69).set(58).set(45), // 57
	BoardMask().set(45).set(57).set(69).set(70).set(59).set(46), // 58
	BoardMask().set(46).set(58).set(70).set(71).set(60).set(47), // 59
	BoardMask().set(47).set(59).set(71).set(72).set(61).set(48), // 60
	BoardMask().set(49).set(48).set(60).set(72), // 61
	BoardMask().set(50).set(51).set(63).set(73), // 62
	BoardMask().set(51).set(62).set(73).set(74).set(64).set(52), // 63
	BoardMask().set(52).set(63).set(74).set(75).set(65).set(53), // 64
	BoardMask().set(53).set(64).set(75).set(76).set(66).set(54), // 65
	BoardMask().set(54).set(65).set(76).set(77).set(67).set(55), // 66
	BoardMask().set(55).set(66).set(77).set(78).set(68).set(56), // 67
	BoardMask().set(56).set(67).set(78).set(79).set(69).set(57), // 68
	BoardMask().set(57).set(68).set(79).set(80).set(70).set(58), // 69
	BoardMask().set(58).set(69).set(80).set(81).set(71).set(59), // 70
	BoardMask().set(59).set(70).set(81).set(82).set(72).set(60), // 71
	BoardMask().set(61).set(60).set(71).set(82), // 72
	BoardMask().set(62).set(63).set(74).set(83), // 73
	BoardMask().set(63).set(73).set(83).set(84).set(75).set(64), // 74
	BoardMask().set(64).set(74).set(84).set(85).set(76).set(65), // 75
	BoardMask().set(65).set(75).set(85).set(86).set(77).set(66), // 76
	BoardMask().set(66).set(76).set(86).set(87).set(78).set(67), // 77
	BoardMask().set(67).set(77).set(87).set(88).set(79).set(68), // 78
	BoardMask().set(68).set(78).set(88).set(89).set(80).set(69), // 79
	BoardMask().set(69).set(79).set(89).set(90).set(81).set(70), // 80
	BoardMask().set(70).set(80).set(90).set(91).set(82).set(71), // 81
	BoardMask().set(72).set(71).set(81).set(91), // 82
	BoardMask().set(73).set(74).set(84).set(92), // 83
	BoardMask().set(74).set(83).set(92).set(93).set(85).set(75), // 84
	BoardMask().set(75).set(84).set(93).set(94).set(86).set(76), // 85
	BoardMask().set(76).set(85).set(94).set(95).set(87).set(77), // 86
	BoardMask().set(77).set(86).set(95).set(96).set(88).set(78), // 87
	BoardMask().set(78).set(87).set(96).set(97).set(89).set(79), // 88
	BoardMask().set(79).set(88).set(97).set(98).set(90).set(80), // 89
	BoardMask().set(80).set(89).set(98).set(99).set(91).set(81), // 90
	BoardMask().set(82).set(81).set(90).set(99), // 91
	BoardMask().set(83).set(84).set(93).set(100), // 92
	BoardMask().set(84).set(92).set(100).set(101).set(94).set(85), // 93
	BoardMask().set(85).set(93).set(101).set(102).set(95).set(86), // 94
	BoardMask().set(86).set(94).set(102).set(103).set(96).set(87), // 95
	BoardMask().set(87).set(95).set(103).set(104).set(97).set(88), // 96
	BoardMask().set(88).set(96).set(104).set(105).set(98).set(89), // 97
	BoardMask().set(89).set(97).set(105).set(106).set(99).set(90), // 98
	BoardMask().set(91).set(90).set(98).set(106), // 99
	BoardMask().set(92).set(93).set(101), // 100
	BoardMask().set(100).set(93).set(94).set(102), // 101
	BoardMask().set(101).set(94).set(95).set(103), // 102
	BoardMask().set(102).set(95).set(96).set(104), // 103
	BoardMask().set(103).set(96).set(97).set(105), // 104
	BoardMask().set(104).set(97).set(98).set(106), // 105
	BoardMask().set(105).set(98).set(99) // 106
};

const BoardMask BoardMask::fullBoard(0xfffffffffffffffe, 0x7ffffffffff);

const BoardMask BoardMask::borders[5] {
	BoardMask().set(1).set(4).set(9).set(16).set(25).set(36).set(49),
	BoardMask().set(49).set(61).set(72).set(82).set(91).set(99).set(106),
	BoardMask().set(106).set(105).set(104).set(103).set(102).set(101).set(100),
	BoardMask().set(100).set(92).set(83).set(73).set(62).set(50).set(37),
	BoardMask().set(37).set(26).set(17).set(10).set(5).set(2).set(1)
};

inline BoardMask BoardPoint::mask() const
{
	return BoardMask(*this);
}

inline BoardMask BoardPoint::neighbors() const
{
	assert(isValid());
	return _neighbors[position() - 1];
}

inline BoardMask::BoardMask(BoardPoint point)
: _a(point.position() <= 63 ? (1ULL << point.position()) : 0)
, _b(point.position() <= 63 ? 0 : (1ULL << (point.position() - 64)))
{
}

inline BoardMask::Itterator BoardMask::itterator() const
{
	return Itterator(*this);
}

BoardMask BoardMask::expanded() const
{
	BoardMask result(*this);
	for(Itterator i(*this); i; ++i)
		result |= i->neighbors();
	return result;
}

BoardMask BoardMask::connected(const BoardMask& seed) const
{
	assert(isValid() && seed.isValid());
	BoardMask result = *this & seed;
	BoardMask border = result;
	while(border) {
		BoardMask nextBorder;
		for(auto i = border.itterator(); i; ++i)
			nextBorder |= i->neighbors() & *this;
		nextBorder -= result;
		result |= nextBorder;
		border = nextBorder;
	}
	return result;
}

vector<BoardMask> BoardMask::groups() const
{
	vector<BoardMask> result;
	BoardMask copy = *this;
	while(copy) {
		BoardMask group = copy.connected(copy.firstPoint());
		result.push_back(group);
		copy -= group;
	}
	return result;
}

uint BoardMask::controlledCorners() const
{
	uint corners = 0;
	
	// Iterate connected groups
	BoardMask remaining(*this);
	while(remaining) {
		// Find a group
		BoardMask group(remaining.firstPoint());
		group = remaining.connected(group);
		remaining -= group;
		
		// See which borders are connected
		uint borders = 0;
		for(uint i = 0; i < 5; ++i)
			if(group & BoardMask::borders[i])
				borders |= 1 << i;
		
		// If three borders are connected, any adjacent borders have the corner controlled
		if(::popcount(borders) >= 3)
			corners |= borders & ((borders >> 1) | (borders << 4));
	}
	return ::popcount(corners);
}

inline BoardPoint BoardMask::firstPoint() const
{
	if(_a)
		return BoardPoint(trailingZeros(_a));
	else if(_b)
		return BoardPoint(64 + trailingZeros(_b));
	else
		return BoardPoint();
}

BoardPoint BoardMask::randomPoint() const
{
	uint p = popcount();
	if(p == 0)
		return BoardPoint();
	uint n = rand() % p;
	Itterator i(*this);
	while(n--)
		i++;
	return *i;
}

class Board {
public:
	Board(): _whiteToPlay(true), _white(), _black() { }
	~Board() { }
	bool operator==(const Board& other) const { return _white == other._white && _black == other._black; }
	
	BoardMask moves() const { return ~(_white | _black); }
	bool gameOver() const;
	void playMove(Move move);
	void playSwapMove();
	
	BoardMask white() const { return _white; }
	BoardMask black() const { return _black; }
	
	int player() const { return _whiteToPlay ? 1 : 2; }
	int winner() const;
	
	void bambooBridges();
	void randomFillUp();
	
protected:
	bool _whiteToPlay;
	BoardMask _white;
	BoardMask _black;
};

std::ostream& operator<<(std::ostream& out, const Board& board)
{
	for(uint i = 1; i <= 106; ++i) {
		bool white = board.white().isSet(i);
		bool black = board.black().isSet(i);
		if(!white && !black)
			out << ".";
		if(white && !black)
			out << "W";
		if(!white && black)
			out << "B";
		if(white && black)
			out << "!";
	}
	return out;
}

void Board::playMove(Move move)
{
	if(_whiteToPlay)
		_white.set(move);
	else
		_black.set(move);
	_whiteToPlay = !_whiteToPlay;
}

void Board::playSwapMove()
{
	assert(!_whiteToPlay);
}

bool Board::gameOver() const
{
	if(!moves())
		return true;
	if(_white.controlledCorners() >= 3)
		return true;
	if(_black.controlledCorners() >= 3)
		return true;
	return false;
}

int Board::winner() const
{
	if(_white.controlledCorners() >= 3)
		return 1;
	if(_black.controlledCorners() >= 3)
		return 2;
	return 0;
}

// Connects bamboo bridges randomly
void Board::bambooBridges()
{
	BoardMask free = ~(_black | _white);
	
	// Find groups and extensions
	vector<BoardMask> whiteGroups = _white.groups();
	vector<BoardMask> whiteExtensions;
	for(const BoardMask& g: whiteGroups)
		whiteExtensions.push_back(g.expanded());
	vector<BoardMask> blackGroups = _black.groups();
	vector<BoardMask> blackExtensions;
	for(const BoardMask& g: blackGroups)
		blackExtensions.push_back(g.expanded());
	
	// Randomly play black or white
	uint whiteIndex = 0;
	uint blackIndex = 0;
	while((whiteIndex < whiteGroups.size()) || (blackIndex < blackGroups.size())) {
		uint i;
		BoardMask group;
		BoardMask* player = nullptr;
		BoardMask* opponent = nullptr;
		vector<BoardMask>* self = nullptr;
		if(whiteIndex != whiteGroups.size() && (blackIndex == blackGroups.size() || (rand() % 2))) {
			i = whiteIndex++;
			group = whiteGroups[i];
			player = &_white;
			opponent = &_black;
			self = &whiteExtensions;
		} else {
			i = blackIndex++;
			group = blackGroups[i];
			player = &_black;
			opponent = &_white;
			self = &blackExtensions;
		}
		BoardMask extension = self->at(i);
		
		// Borders
		for(uint j = 0; j < 5; j++) {
			BoardMask border = BoardMask::borders[j];
			if(group & border)
				continue; // Already connected to border
			BoardMask bamboo = extension & border & free;
			if(bamboo.popcount() == 2) {
				BoardPoint p = bamboo.randomPoint();
				player->set(p);
				*opponent |= bamboo - *player;
				free -= bamboo;
			}
		}
		
		// Other groups
		for(uint j = 0; j < i; j++) {
			BoardMask bamboo = extension & self->at(j) & free;
			if(bamboo.popcount() == 2) {
				BoardPoint p = bamboo.randomPoint();
				player->set(p);
				*opponent |= bamboo - *player;
				free -= bamboo;
			}
		}
	}
}

void Board::randomFillUp()
{
	BoardMask free = ~(_black | _white);
	uint whiteStones = 53 - _black.popcount();
	uint blackStones = 53 - _white.popcount();
	
	// Fill up with equal amounts of stones
	for(auto i = free.itterator(); i; i++) {
		uint dice = rand() % (blackStones + whiteStones);
		if(dice < whiteStones) {
			_white.set(*i);
			whiteStones--;
		} else {
			_black.set(*i);
			blackStones--;
		}
	}
	assert(whiteStones == 0 && blackStones == 0);
}

class TreeNode {
public:
	static constexpr int nActions = 5;
	static constexpr double epsilon = 1e-6;
	static constexpr double explorationParameter = sqrt(2.0);
	static uint numNodes() { return _numNodes; }
	
	TreeNode(): TreeNode(Board()) {}
	TreeNode(const Board& board);
	~TreeNode();
	
	void loadGames(const string& file);
	
	void selectAction();
	void expand();
	bool isLeaf() { return _children.empty(); }
	double rollOut() const;
	void updateStats(double value);
	uint arity() { return _children.size(); }
	
	TreeNode* child(Move move);
	
	// Favorite child, forget all other children
	void vincent(TreeNode* child);
	
	double visits() const { return _visits; }
	double totalValue() const { return _totalValue; }
	
	Move bestMove() const;
	
protected:
	static uint _numNodes;
	Board _board;
	double _visits;
	double _totalValue;
	vector<TreeNode*> _children;
	TreeNode* select() const;
};

uint TreeNode::_numNodes = 0;

TreeNode::TreeNode(const Board& board)
: _board(board)
, _visits(0.0)
, _totalValue(0.0)
, _children()
{
	_numNodes++;
}

TreeNode::~TreeNode()
{
	for(TreeNode* node: _children)
		delete node;
	_numNodes--;
}

void TreeNode::loadGames(const string& filename)
{
	ifstream file(filename);
	if(!file.good()) {
		cerr << "Could not read: " << filename << endl;
		return;
	}
	for(string line; getline(file, line); ) {
		cerr << "Tree: " << TreeNode::numNodes() << endl;
		cerr << "Game: ";
		stringstream ss(line);
		
		TreeNode* gameState = this;
		while(ss.good()) {
			Move move;
			ss >> move;
			assert(move.isValid());
			cerr << "[" << move << "]";
			gameState = gameState->child(move);
			assert(gameState);
		}
		assert(gameState->_board.gameOver());
		cerr << endl;
	}
}

TreeNode* TreeNode::child(Move move)
{
	expand(); /// @todo A full expand may be to much
	Board resultingBoard(_board);
	resultingBoard.playMove(move);
	for(TreeNode* c: _children) {
		if(c->_board == resultingBoard)
			return c;
	}
	return nullptr;
}

void TreeNode::vincent(TreeNode* child)
{
	// Forget other children
	for(TreeNode* c: _children)
		if(c != child)
			delete c;
	_children.clear();
	_children.push_back(child);
}

void TreeNode::selectAction()
{
	vector<TreeNode*> visited;
	TreeNode* current = this;
	visited.push_back(current);
	while(!current->isLeaf()) {
		current = current->select();
		visited.push_back(current);
	}
	current->expand();
	if(!current->isLeaf()) {
		TreeNode* newNode = current->select();
		assert(newNode);
		visited.push_back(newNode);
		current = newNode;
	}
	double value = current->rollOut();
	for(TreeNode* node: visited) {
		if(node->_board.player() == current->_board.player())
			node->updateStats(value);
		else
			node->updateStats(1.0 - value);
	}
}

TreeNode* TreeNode::select() const
{
	TreeNode* selected = nullptr;
	double bestValue = 0.0;
	const double logParentVisits = log(visits() + 1);
	for(TreeNode* c: _children) {
		double uctValue =
			c->totalValue() / (c->visits() + epsilon) +
			explorationParameter * sqrt(logParentVisits / (c->visits() + epsilon)) +
			randomReal() * epsilon; // small random number to break ties randomly in unexpanded nodes
		if(uctValue > bestValue) {
			selected = c;
			bestValue = uctValue;
		}
	}
	return selected;
}

Move TreeNode::bestMove() const
{
	// Find node with highest playout count
	TreeNode* best = nullptr;
	uint value = 0;
	for(TreeNode* c: _children) {
		double v = c->_totalValue / (c->_visits + epsilon);
		assert(v >= 0.0);
		assert(v <= 1.0);
		if(v > value) {
			best = c;
			value = v;
		}
	}
	assert(best);
	BoardMask moveMask = _board.moves() - best->_board.moves();
	assert(moveMask.popcount() == 1);
	return moveMask.firstPoint();
}

void TreeNode::expand()
{
	if(!_children.empty())
		return;
	if(_board.gameOver())
		return;
	BoardMask moves = _board.moves();
	for(BoardMask::Itterator i(moves); i; ++i) {
		Board next(_board);
		next.playMove(*i);
		_children.push_back(new TreeNode(next));
	}
}

double TreeNode::rollOut() const
{
	const uint bambooRepeats = 3;
	const uint fillOutRepeats = 4;
	double result = 0.0;
	
	// Do the bamboo bridges
	for(uint b = 0; b < bambooRepeats; b++) {
		// Create bamboo bridges
		Board bamboo(_board);
		bamboo.bambooBridges();
		
		// Early exit if already won
		int winner = bamboo.winner();
		if(winner != 0) {
			result += ((winner == _board.player()) ? 0.0 : 1.0) * fillOutRepeats;
			continue;
		}
		
		// Do the fill-outs
		for(uint i = 0; i < fillOutRepeats; i++) {
			Board fillOut(bamboo);
			fillOut.randomFillUp();
			
			// 0, ½ or 1 point
			int winner = fillOut.winner();
			if(winner == 0)
				result += 0.5;
			if(winner == _board.player())
				result += 0.0;
			else
				result += 1.0;
		}
	}
	return result / (fillOutRepeats * bambooRepeats);
}

void TreeNode::updateStats(double value)
{
	++_visits;
	_totalValue += value;
}

class GameInputOutput {
public:
	GameInputOutput(): _board(), _tree(new TreeNode(_board)) { }
	~GameInputOutput() { delete _tree; }
	void run();
	void playMove(Move move);
	Move generateMove();
	
protected:
	Board _board;
	TreeNode* _tree;
};

void GameInputOutput::run()
{
	/// @todo Opening book
	
	// First move
	bool isWhite = false;
	string line;
	cin >> line;
	cerr << "In: " << line << endl;
	if(line == "Start") {
		isWhite = true;
		BoardPoint move = generateMove();
		playMove(move);
		cerr << _board << endl;
		cerr << "Out: " << move << endl;
		cout << move << endl;
	} else {
		isWhite = false;
		playMove(line);
	}
	cerr << _board << endl;
	
	// Second move
	if(isWhite) {
		cin >> line;
		cerr << "In: " << line << endl;
		if(line == "-1")
			isWhite = false;
		else
			playMove(line);
	} else {
		bool swap = true; /// Todo
		if(swap) {
			cerr << "Out: -1" << endl;
			cout << "-1" << endl;
			isWhite = true;
		} else {
			BoardPoint move = generateMove();
			playMove(move);
			cerr << _board << endl;
			cerr << "Out: " << move << endl;
			cout << move << endl;
		}
		
		// Handle move 3 to enter main loop
		cin >> line;
		cerr << "In: " << line << endl;
		playMove(line);
	}
	cerr << _board << endl;
	
	// Game loop
	for(;;) {
		BoardPoint move = generateMove();
		playMove(move);
		cerr << _board << endl;
		cerr << "Out: " << move << endl;
		cout << move << endl;
		if(_board.gameOver())
			break;
		cin >> line;
		cerr << "In: " << line << endl;
		playMove(line);
		if(_board.gameOver())
			break;
	}
	cin >> line;
	cerr << "In: " << line << endl;
	cerr << "Quiting" << line << endl;
}

Move GameInputOutput::generateMove()
{
	for(uint i = 0; i < 7500; ++i)
		_tree->selectAction();
	cerr << "nodes: " << TreeNode::numNodes()  << " (" << _tree->visits() << " visits)" << " (";
	cerr << (TreeNode::numNodes() * sizeof(TreeNode) / (1024*1024))  << " MB)" <<endl;
	return _tree->bestMove();
}

void GameInputOutput::playMove(Move move)
{
	_board.playMove(move);
	TreeNode* newTree = _tree->child(move);
	_tree->vincent(newTree);
	_tree = newTree;
	cerr << "nodes: " << TreeNode::numNodes() << " (" << _tree->visits() << " visits)"  << endl;
}

int main(int argc, char* argv[])
{
	cerr << "R " << argv[0]  << endl;
	srand(time(0));
	
	TreeNode tn;
	tn.loadGames("competitions-sym.txt");
	
	return 0;
	
	GameInputOutput gio;
	gio.run();
	cerr << "Exit" << endl;
	return 0;
}
