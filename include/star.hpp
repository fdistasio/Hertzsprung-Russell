#ifndef STAR_HPP
#define STAR_HPP

// Star object: absolute magnitude and color index
class Star {

    public:
    
        Star();
        Star(double absoluteMagnitude, double colorIndex);
        
        double getAbs();
        double getCi();
    
    private:

        double absoluteMagnitude;
        double colorIndex;

};

#endif