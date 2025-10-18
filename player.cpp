#include "player.h"
Player::Player() : name("Player"), score(0), isBot(false) {}
Player::Player(QString name_, bool isBot_) : name(name_), score(0), isBot(isBot_) {}
