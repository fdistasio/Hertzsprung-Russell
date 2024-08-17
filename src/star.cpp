#include "../include/star.hpp"

// Star object: absolute magnitude and color index
Star::Star():
    absoluteMagnitude {-1}, colorIndex {-1} {}

Star::Star(double absoluteMagnitude, double colorIndex):
    absoluteMagnitude {absoluteMagnitude}, colorIndex {colorIndex} {}

double Star::getAbs() {

    return this->absoluteMagnitude;
}

double Star::getCi() {

    return this->colorIndex;
}