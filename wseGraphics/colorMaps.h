#ifndef _colorMap_h
#define _colorMap_h

#include <vector>
#include <string>
#include <QColor>

class colorMap
{
public:

  std::string name;
  std::vector<QColor> colors;

  colorMap()  {}
  ~colorMap() {}
};

class colorMaps : public std::vector<colorMap>
{
public:

  colorMaps()
  {    
    colorMap a;

    QColor brown(205,133,63);

    //a.name = "original";
    //a.colors.clear();
    //a.colors.push_back(Qt::white);
    //a.colors.push_back(Qt::white);
    //this->push_back(a);

    a.name = "Rainbow";
    a.colors.clear();
    a.colors.push_back(Qt::blue);
    a.colors.push_back(Qt::cyan);
    a.colors.push_back(Qt::green);
    a.colors.push_back(Qt::yellow);
    a.colors.push_back(Qt::red);
    this->push_back(a);

    a.name = "Blue to Green";
    a.colors.clear();
    a.colors.push_back(Qt::blue);
    a.colors.push_back(Qt::cyan);
    a.colors.push_back(Qt::green);
    this->push_back(a);

    a.name = "Grayscale";
    a.colors.clear();
    a.colors.push_back(Qt::black);
    a.colors.push_back(Qt::white);
    this->push_back(a);

    a.name = "Cool to Warm";
    a.colors.clear();
    a.colors.push_back(Qt::blue);
    a.colors.push_back(Qt::white);
    a.colors.push_back(Qt::red);
    this->push_back(a);

    a.name = "Black-Body";
    a.colors.clear();
    a.colors.push_back(Qt::black);
    a.colors.push_back(Qt::red);
    a.colors.push_back(Qt::yellow);
    a.colors.push_back(Qt::white);
    this->push_back(a);

    a.name = "Elevation";
    a.colors.clear();
    a.colors.push_back(Qt::darkGreen);
    //a.colors.push_back(brown);
    a.colors.push_back(Qt::darkYellow);
    a.colors.push_back(Qt::yellow);
    a.colors.push_back(Qt::white);
    this->push_back(a);

    a.name = "Fire";
    a.colors.clear();
    a.colors.push_back(Qt::blue);
    a.colors.push_back(Qt::cyan);
    a.colors.push_back(Qt::green);
    a.colors.push_back(Qt::magenta);
    a.colors.push_back(Qt::red);
    a.colors.push_back(Qt::yellow);
    a.colors.push_back(Qt::white);
    this->push_back(a);

    a.name = "Contrast #1";
    a.colors.clear();
    a.colors.push_back(Qt::blue);
    a.colors.push_back(Qt::yellow);
    a.colors.push_back(Qt::magenta);
    a.colors.push_back(Qt::green);
    a.colors.push_back(Qt::red);
    a.colors.push_back(Qt::cyan);
    a.colors.push_back(Qt::white);
    this->push_back(a);

    a.name = "Contrast #2";
    a.colors.clear();
    a.colors.push_back(Qt::blue);
    a.colors.push_back(Qt::magenta);
    a.colors.push_back(Qt::green);
    a.colors.push_back(Qt::yellow);
    a.colors.push_back(Qt::cyan);
    a.colors.push_back(Qt::red);
    a.colors.push_back(Qt::white);
    this->push_back(a);

    a.name = "Contrast #3";
    a.colors.clear();
    a.colors.push_back(Qt::black);
    a.colors.push_back(Qt::green);
    a.colors.push_back(Qt::darkBlue);
    a.colors.push_back(Qt::yellow);
    a.colors.push_back(Qt::red);
    a.colors.push_back(Qt::white);
    a.colors.push_back(Qt::magenta);
    this->push_back(a);

    
  }
  ~colorMaps() {}
  
};


#endif
