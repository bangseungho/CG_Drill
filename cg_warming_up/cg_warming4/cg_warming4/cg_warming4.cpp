#define PIXEL 50

#include <iostream>

using namespace std;

class Rectangle
{
public:
    Rectangle(int Index)
        : _index(Index)
    {

    }
    
    void init()
    {
        while (true)
        {
            _left = rand() % 800 + 1;
            _top = rand() % 600 + 1;
            _right = rand() % 800 + 1;
            _bottom = rand() % 600 + 1;

            if ((_left < _right) && (_top < _bottom))
                break;
        }
    }

    void printinfo()
    {
        cout << "           Rect " << _index << ": " << "(" << _left << ", " << _top << ")" << "(" << _right << ", " << _bottom << ")" << endl;
    }

    void move(int x, int y)
    {
        _left += x;
        _top += y;
        _right += x;
        _bottom += y;

        if (_left < 0 || _top < 0 || _right > 800 || _bottom > 600)
        {
            _left -= x;
            _top -= y;
            _right -= x;
            _bottom -= y;
            cout << "#ERROR Rectangle " << _index << " is get out!" << endl << endl;
        }
    }

    Rectangle* get_pos()
    {
        return this;
    }

public:
    int _index = 0;
    int _left = 0;
    int _top = 0; 
    int _right = 0;
    int _bottom = 0;
};

void collide(const Rectangle& r1, const Rectangle& r2)
{
    if (r1._left < r2._right && r1._right > r2._left && 
        r1._top < r2._bottom && r1._bottom > r2._top)
        cout << "Rectangle 1 & Rectangle 2 collider!" << endl << endl;
}

int main()
{
    srand(static_cast<unsigned int>(time(nullptr)));

    Rectangle rec1(1);
    Rectangle rec2(2);
    
    rec1.init();
    rec2.init();

    cout << "Input Shape Coordinates value: " << endl << endl;
    rec1.printinfo();
    cout << endl;
    rec2.printinfo();

    char input;

    while (1)
    {
        cout << "Command: ";
        cin >> input;
        cout << endl;

        if (input == 'q')
            break;

        switch (input)
        {
        case 'w':
            rec1.move(0, -PIXEL);
            break;
        case 'a':
            rec1.move(-PIXEL, 0);
            break;
        case 's':
            rec1.move(0, PIXEL);
            break;
        case 'd':
            rec1.move(PIXEL, 0);
            break;
        case 'i':
            rec2.move(0, -PIXEL);
            break;
        case 'j':
            rec2.move(-PIXEL, 0);
            break;
        case 'k':
            rec2.move(0, PIXEL);
            break;
        case 'l':
            rec2.move(PIXEL, 0);
            break;
        }

        collide(rec1, rec2);

        rec1.printinfo();
        cout << endl;
        rec2.printinfo();
    }

    return 0;
}
