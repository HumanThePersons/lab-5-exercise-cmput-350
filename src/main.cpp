#include <iostream>
#include <optional>
#include <vector>
#include <cmath>

#include <SFML/Graphics.hpp>

const int WINDOW_WIDTH = 800;
const int WINDOW_HEIGHT = 800;
const int FPS_LIMIT = 30;
const float PI = 3.141592653589793;

using Point2D = sf::Vector2f;

// TODO: (Part 1) Define a function that samples a cubic Bezier curve at t in [0, 1].
Point2D getPoint(const std::vector<sf::Vector2f>& pts, float t) { 
    // pts is a pointer to a vector containing the point vectors
    // Create a temp vector to hold the lerps
    std::vector<sf::Vector2f> temp;
    temp.clear();

    // To iterate through the given points in pts
    for (int x = 0; x < pts.size() - 1; ++x) {
        temp.push_back(pts[x]*(1.0f - t) + pts[x + 1] * t);  // lerp - (1 - t) * a + t * b
    }

    // Work thought each point in temp
    while (temp.size() > 1) {
        for (int x = 0; x < temp.size() - 1; ++x) {
            temp[x] = temp[x]*(1.0f - t) + temp[x + 1] * t;
        }
        temp.pop_back();  // Remove points until all are handled
    }

    return temp[0];
}


// TODO: (Part 2) Define a function that returns the curve's slope at t in [0, 1].
Point2D getSlope(const std::vector<sf::Vector2f>& pts, float t) { 
    // Derivative of the curve
    Point2D slopeCurve = Point2D();

    // 3 * (1 - t)^2 * (p2 - p1)
    Point2D tempPoint = (pts[1] - pts[0]);
    slopeCurve.x = (tempPoint.x)*(3 * std::pow(1-t, 2));
    slopeCurve.y = (tempPoint.y)*(3 * std::pow(1-t, 2));

    // 6 * (1 - t) * t * (p3 - p2)
    tempPoint = (pts[2] - pts[1]);
    slopeCurve.x += (tempPoint.x)*(6*(1 - t)*t);
    slopeCurve.y += (tempPoint.y)*(6*(1 - t)*t);

    // 3 * t^2 * (p4 - p3)
    tempPoint = (pts[3] - pts[2]);
    slopeCurve.x += (tempPoint.x)*(3*std::pow(t, 2));
    slopeCurve.y += (tempPoint.y)*(3*std::pow(t, 2));

    return slopeCurve; 
}

// TODO: (Part 1) Store four control points for the curve.
float pointRadius = 6.0f;
std::vector<Point2D> points(4);
std::vector<sf::CircleShape> pointShapes(4);


// TODO: (Part 2) Track animation time for the square moving along the curve.
int animationFrames = 60;
int currFrame = 0;

// TODO: (Part 3) Track the index of the control point being dragged.
int selectedPoint = -1;   // Part 3: index being dragged, -1 = none
void movePoint(int index, Point2D position) {
    points[index] = position;
    pointShapes[index].setPosition(position);
}

void handleInput(sf::Window& window, bool& shouldQuit) {
    while (const std::optional<sf::Event> event = window.pollEvent()) {
        if (event->is<sf::Event::Closed>()) {
            window.close();
            shouldQuit = true;
        } else if (const auto* mouse = event->getIf<sf::Event::MouseButtonPressed>()) {
            // TODO: (Part 3) On left-click, select the closest control point
            // using mouse->position and start dragging it.
            // use mouse->position
            switch (mouse->button) {
                case sf::Mouse::Button::Right:
                    break;
                case sf::Mouse::Button::Left:
                {
                    // Select nearest control point
                    Point2D mousePoint(mouse->position);

                    float minDistance = 100000.0f;

                    for (int i = 0; i < 4; ++i){
                        // iterate through the points and find the one closest to where the mouse clicked
                        float distance = std::sqrt(std::pow((points[i].x - mousePoint.x), 2) + 
                            std::pow((points[i].y - mousePoint.y), 2));
                        if (distance < minDistance){
                            minDistance = distance;
                            selectedPoint = i;
                        }        
                    }
                    // Let the point follow the mouse
                    movePoint(selectedPoint, mousePoint);
                    break;
                }
                case sf::Mouse::Button::Middle:
                    break;
                default: 
                    break;
            }
        } else if (const auto* mouse = event->getIf<sf::Event::MouseButtonReleased>()) {
            // TODO: (Part 3) On left-button release, stop dragging.
            if (mouse->button == sf::Mouse::Button::Left) {
                selectedPoint = -1;   // stop dragging
            }

        } else if (const auto* mouse = event->getIf<sf::Event::MouseMoved>()) {
            // TODO: (Part 3) Move the selected control point to mouse->position.
            // TODO: (Part 4) Maintain matching slopes at shared endpoints.
            // When moving point 3, move point 5 without changing its distance
            // from point 4 (point numbers here start at 1).
            if (selectedPoint != -1) {
                movePoint(selectedPoint, Point2D(mouse->position));
            }
        } else if (const auto* key = event->getIf<sf::Event::KeyPressed>()) {
            // TODO: (Part 4) '+' adds three control points; '-' removes three,
            // keeping at least four points.
        }
    }
}

/*
Implement a mouse handler that, when you left-click, finds the closest point on the Bézier curve 
and updates its location to follow the mouse. As you move the mouse, the curve should be updated 
dynamically. You should keep track of the current point being manipulated once the mouse is clicked 
down, and that point should be continuously manipulated until a mouse up event is received. (Tip: 
keep an integer with the index in the vector of points that is being edited, and then manipulate 
the point in that location when any mouse events occur. When editing stops, reset that integer to 
ensure no further editing occurs. That is, if you are moving the third point, keep an integer with 
the value 3 to reference which point is currently being edited.)
*/

void DrawLine(sf::RenderWindow& window, Point2D from, Point2D to, float width = 2.0f) {
    // Construct shape with 4 points - calculate 2 along the perpendicular line at the end of line that runs through
    // 'from' and calculate the other 2 along the perpendicular line at the end of the line that runs through 'to'
    sf::ConvexShape drawableLine = sf::ConvexShape(4); // constructor (std::size_t pointCount=0) - set 4 points

    // end.x is the change in y between 'to' an 'from', end.x is the *negative* change in y
    Point2D end = Point2D(from.y - to.y, to.x - from.x);  // -(y2 - y1) = y1 - y2
    
    // Normalize end then multiply by w / 2
    end = end.normalized();
    end *= (width / 2.0f); 

    // 4 points, two calculated from each 'from' and 'to'
    sf::Vector2f p1 = sf::Vector2f(from.x - end.x, from.y - end.y);  // p1 = from - end
    sf::Vector2f p2 = sf::Vector2f(from.x + end.x, from.y + end.y);  // p2 = from + end
    sf::Vector2f p3 = sf::Vector2f(to.x + end.x, to.y + end.y);  // p3 = to + end
    sf::Vector2f p4 = sf::Vector2f(to.x - end.x, to.y - end.y);  // p4 = to - end

    // Set these 4 points to make up drawableLine
    drawableLine.setPoint(0, p1);
    drawableLine.setPoint(1, p2);
    drawableLine.setPoint(2, p3);
    drawableLine.setPoint(3, p4);

    // Set colour
    drawableLine.setFillColor(sf::Color::White);

    // Draw the line
    window.draw(drawableLine);
}

void initializePoints() { 
    // Populate shapes
    for (int i = 0; i < 4; ++i) {
        pointShapes[i] = sf::CircleShape(pointRadius);
        pointShapes[i].setFillColor(sf::Color::Cyan);
        pointShapes[i].setOrigin(sf::Vector2f(pointRadius, pointRadius));
    }

    // Initialize start points
    points[0] = Point2D(100.0f, 100.0f);
    points[1] = Point2D(50.0f, 200.0f);
    points[2] = Point2D(500.0f, 400.0f);
    points[3] = Point2D(300.0f, 80.0f);

    pointShapes[0].setPosition(points[0]);
    pointShapes[1].setPosition(points[1]);
    pointShapes[2].setPosition(points[2]);
    pointShapes[3].setPosition(points[3]);
}

void render(sf::RenderWindow& window) {
    window.clear(sf::Color::Black);
    // ====== ====== ======
    // TODO: (Part 1) Sample GetPoint over t in [0, 1] and connect samples using the line-drawing
    // code from your project. Draw all four control points as circles after drawing the curve.
    // ====== ====== ======

    const int samples = 200;
    for (int sampleNum = 0; sampleNum < samples; sampleNum++) {
        // Iterate over all the samples
        float tVal = sampleNum / float(samples);  // sampleNum normalized to fit in [0, 1]
        float tNextVal = (sampleNum + 1) / float(samples);
        
        Point2D curvePoint1 = getPoint(points, tVal);  // Gets current and next point
        Point2D curvePoint2 = getPoint(points, tNextVal);

        // Draw a line between them
        DrawLine(window, curvePoint1, curvePoint2);
    }

    for (int i = 0; i < 4; i++) {
        // Draw a circle for each point
        window.draw(pointShapes[i]);
    };

    // ====== ====== ======
    // TODO: (Part 2) Draw a small square moving repeatedly along the curve.
    // Use GetSlope to orient it to the curve at each time step.
    // ====== ====== ======

    // Start by drawing the square on the curve
    float squareSize = 10.0f;
    sf::RectangleShape square = sf::RectangleShape(Point2D(squareSize, squareSize));
    square.setOrigin(sf::Vector2f(squareSize / 2.0f, squareSize / 2.0f));

    // Get the t between 0 to 1 from the frames - % makes it reset to 0 for the cycle, dividing normalizes it
    float t = (currFrame % animationFrames) / float(animationFrames);
    Point2D squareLocation = getPoint(points, t);
    square.setPosition(squareLocation);

    // The squares rotation at any given time should be perpendicular to the slope
    // Slopes angle relative to x axis:
    Point2D slope = getSlope(points, t);
    float angle = std::atan(slope.y / slope.x);  // This angle is in radians
    angle *= (180 / PI); // 180 / π to convert to degrees

    // The squares angle is the angle of the slope + 90 degrees (perpendicular to slope)
    square.setRotation(sf::degrees(angle + 90));

    ++currFrame;
    window.draw(square);

    // ====== ====== ======
    // TODO: (Part 3) Draw control handles from point 1 to 2 and point 3 to 4.
    // TODO: (Part 4) Draw all connected cubic Bezier segments and their handles.
    // ====== ====== ======

    // Draw the control handles
    DrawLine(window, points[0], points[1]);
    DrawLine(window, points[2], points[3]);

    // ====== ====== ======
    // TODO: (Bonus) Support multiple curves, a Galaga screen overlay at a 1:2 ratio, and exporting
    // curve points as C++ code for Project 1b.
    // ====== ====== ======

    window.display();
}

int main() {
    sf::RenderWindow window;

    try {
        // Initialize window
        window.create(sf::VideoMode({WINDOW_WIDTH, WINDOW_HEIGHT}), "Bezier Curve Editor");
        window.setFramerateLimit(FPS_LIMIT);
        initializePoints();  // To create start points and shapes

        // Prevent key repeats.
        window.setKeyRepeatEnabled(false);

        bool shouldQuit = false;
        // Main game loop
        while (window.isOpen()) {
            handleInput(window, shouldQuit);
            if (shouldQuit) {
                break;
            }
            render(window);
        }
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return -1;
    }
    return 0;
}
