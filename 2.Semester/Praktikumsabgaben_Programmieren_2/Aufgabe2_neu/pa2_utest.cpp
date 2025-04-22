#define BOOST_TEST_MAIN
#include <cmath>
#include <type_traits>
#include <vector>
#include <boost/test/included/unit_test.hpp>
#include "ufo.h"
#include "ballistic.h"
#include "vertical.h"

BOOST_AUTO_TEST_SUITE(pa_utest)

std::vector<float> dest1 = { 10.0, 10.0, 8.0 };
std::vector<float> dest2 = { 10.0, 0.0, 10.0 };
std::vector<float> dest3 = { 30.0, 30.0, 8.0 };
std::vector<float> dest4 = { -10.0, 0.0, 4.0 };
std::vector<float> dest5 = { 10.0, 20.0, 8.0 };
std::vector<float> dest6 = { -20.0, -15.0, 3.0 };

BOOST_AUTO_TEST_CASE(vertical_initially)
{
    Vertical vert("r2d2");
    BOOST_CHECK(vert.getId() == "r2d2");

    BOOST_CHECK(size(vert.getPosition()) == 3);

    if (size(vert.getPosition()) == 3)
    {
        BOOST_CHECK(vert.getPosition()[0] == 0.0);
        BOOST_CHECK(vert.getPosition()[1] == 0.0);
        BOOST_CHECK(vert.getPosition()[2] == 0.0);
    }

    BOOST_CHECK(vert.getFtime() == 0.0);
}

BOOST_AUTO_TEST_CASE(vertical_after_one_flight)
{
    Vertical vert("r2d2");
    vert.flyToDest(dest1[0], dest1[1], dest1[2], 10);
    BOOST_CHECK(vert.getId() == "r2d2");

    BOOST_CHECK(size(vert.getPosition()) == 3);

    if (size(vert.getPosition()) == 3)
    {
        BOOST_CHECK(fabs(vert.getPosition()[0] - dest1[0]) < 0.1);
        BOOST_CHECK(fabs(vert.getPosition()[1] - dest1[1]) < 0.1);
        BOOST_CHECK(vert.getPosition()[2] == 0.0);
    }

    BOOST_CHECK(fabs(vert.getFtime() - 38.0) < 3);
}

BOOST_AUTO_TEST_CASE(vertical_after_two_flights)
{
    Vertical vert("r2d2");
    vert.flyToDest(dest1[0], dest1[1], dest1[2], 10);
    vert.flyToDest(dest2[0], dest2[1], dest2[2], 15);
    BOOST_CHECK(vert.getId() == "r2d2");

    BOOST_CHECK(size(vert.getPosition()) == 3);

    if (size(vert.getPosition()) == 3)
    {
        BOOST_CHECK(fabs(vert.getPosition()[0] - dest2[0]) < 0.1);
        BOOST_CHECK(fabs(vert.getPosition()[1] - dest2[1]) < 0.1);
        BOOST_CHECK(vert.getPosition()[2] == 0.0);
    }
}

bool check_waypoint(const float x1, const float y1,
                    const float x2, const float y2,
                    const float h, const float phi)
{
    std::vector<float> B = Ufo::wayPoint(x1, y1, x2, y2, h, phi);
    return fabs((B[0] - x1) * (y2 - y1) - (B[1] - y1) * (x2 - x1) < 0.001);
}

BOOST_AUTO_TEST_CASE(waypoint)
{
    // basic tests
    std::vector<float> B = Ufo::wayPoint(2.0, 0.0, 100.3f, 0.0, 10.0, 45.0);
    BOOST_CHECK(fabs(B[0] - 12.0) < 0.001);
    BOOST_CHECK(fabs(B[1] - 0.0) < 0.001);
    B = Ufo::wayPoint(0.0, -20.0, 0.0, 55.5, 10.0, 45.0);
    BOOST_CHECK(fabs(B[0] - 0.0) < 0.001);
    BOOST_CHECK(fabs(B[1] - -10.0) < 0.001);

    // legacy tests
    B = Ufo::wayPoint(0.0, 0.0, 30.0, 30.0, 8.0, 45.0);
    BOOST_CHECK(fabs(B[0] - 5.65685) < 0.001);
    BOOST_CHECK(fabs(B[1] - 5.65685) < 0.001);
    // 1st quadrant to 1st quadrant, north east, near A (0.188562)
    BOOST_CHECK(check_waypoint(0.0, 0.0, 30.0, 30.0, 8.0, 45.0));

    B = Ufo::wayPoint(0.0, 0.0, 10.0, 20.0, 8.0, 45.0);
    BOOST_CHECK(fabs(B[0] - 3.57771) < 0.001);
    BOOST_CHECK(fabs(B[1] - 7.15542) < 0.001);
    // 1st quadrant to 1st quadrant, north east, between A and D (0.357771)
    BOOST_CHECK(check_waypoint(0.0, 0.0, 10.0, 20.0, 8.0, 45.0));

    // more tests
    // pos. x-axis to pos. x-axis, east, near A (0.101729)
    BOOST_CHECK(check_waypoint(2.0, 0.0, 100.3f, 0.0, 10.0, 45.0));
    // neg. y-axis to pos. y-axis, north, near A (0.13245)
    BOOST_CHECK(check_waypoint(0.0, -20.0, 0.0, 55.5, 10.0, 45.0));
    // 3rd quadrant to 1st quadrant, east, far behind D (238.772)
    BOOST_CHECK(check_waypoint(-11.9f, 1.5, 0.1f, 1.4f, 5.001f, 0.1f));
    // 3rd quadrant to 4th quadrant, south, near A (0.107593)
    BOOST_CHECK(check_waypoint(-11.9f, 1.5, -12.0, -100.0, 35.99f, 73.12f));
    // 3rd quadrant to 3rd quadrant, north, close to A (0.0058055)
    BOOST_CHECK(check_waypoint(-11.9f, 1.5, -11.9f, 1.51f, 3.333f, 89.999f));
    // 4th quadrant to 4th quadrant, north east, close to D (0.977121)
    BOOST_CHECK(check_waypoint(20.0, -20.0, 35.0, -13.7f, 8.1f, 27.0001f));
}

BOOST_AUTO_TEST_CASE(ballistic_angles)
{
    Ballistic ball1("ball1", -10.0, 60.0);
    BOOST_CHECK(ball1.getTakeOffAngle() == 45.0);
    BOOST_CHECK(ball1.getLandingAngle() == 60.0);

    Ballistic ball2("ball2", 0.0, 90.0);
    BOOST_CHECK(ball2.getTakeOffAngle() == 45.0);
    BOOST_CHECK(ball2.getLandingAngle() == 90.0);

    Ballistic ball3("ball3", 35.0, 90.0001f);
    BOOST_CHECK(ball3.getTakeOffAngle() == 35.0);
    BOOST_CHECK(ball3.getLandingAngle() == 45.0);

    Ballistic ball4("ball4", 90.0, 0.0);
    BOOST_CHECK(ball4.getTakeOffAngle() == 90.0);
    BOOST_CHECK(ball4.getLandingAngle() == 45.0);

    Ballistic ball5("ball5", 275.0, -5.0);
    BOOST_CHECK(ball5.getTakeOffAngle() == 45.0);
    BOOST_CHECK(ball5.getLandingAngle() == 45.0);
}

BOOST_AUTO_TEST_CASE(ballistic_polymorphism)
{
    Vertical vert("r2d2");
    Ballistic ball("ball", 45.0, 70.0);
    std::vector<Ufo*> ufos = { &vert, &ball };

    for (Ufo* ufo: ufos)
    {
        ufo->flyToDest(dest3[0], dest3[1], dest3[2], 10);
        BOOST_CHECK(size(ufo->getPosition()) == 3);

        if (size(ufo->getPosition()) == 3)
        {
            BOOST_CHECK(fabs(ufo->getPosition()[0] - dest3[0]) < 0.3);
            BOOST_CHECK(fabs(ufo->getPosition()[1] - dest3[1]) < 0.3);
            BOOST_CHECK(ufo->getPosition()[2] == 0.0);
        }
    }
}

BOOST_AUTO_TEST_CASE(ballistic_waypoints1)
{
    Ballistic ball("ball", 45.0, 70.0);
    std::vector<float> first = ball.firstWaypoint(dest3[0], dest3[1], dest3[2]);
    std::vector<float> second = ball.secondWaypoint(dest3[0], dest3[1], dest3[2]);

    BOOST_CHECK(size(first) == 2);

    if (size(first) == 2)
    {
        BOOST_CHECK(fabs(first[0] - 5.65685) < 0.1);
        BOOST_CHECK(fabs(first[1] - 5.65685) < 0.1);
    }

    BOOST_CHECK(size(second) == 2);

    if (size(second) == 2)
    {
        BOOST_CHECK(fabs(second[0] - 27.9411) < 0.1);
        BOOST_CHECK(fabs(second[1] - 27.9411) < 0.1);
    }
}

BOOST_AUTO_TEST_CASE(ballistic_waypoints2)
{
    Ballistic ball("ball", 45.0, 70.0);
    std::vector<float> first = ball.firstWaypoint(dest5[0], dest5[1], dest5[2]);
    std::vector<float> second = ball.secondWaypoint(dest5[0], dest5[1], dest5[2]);

    BOOST_CHECK(size(first) == 2);

    if (size(first) == 2)
    {
        BOOST_CHECK(fabs(first[0] - 3.57771) < 0.1);
        BOOST_CHECK(fabs(first[1] - 7.15542) < 0.1);
    }

    BOOST_CHECK(size(second) == 2);

    if (size(second) == 2)
    {
        BOOST_CHECK(fabs(second[0] - 8.69782) < 0.1);
        BOOST_CHECK(fabs(second[1] - 17.3956) < 0.1);
    }
}

BOOST_AUTO_TEST_CASE(ballistic_waypoints_90degrees)
{
    // method waypoint, which is called in firstWaypoint and in
    // secondWaypoint, is only defined for angles less than 90 degrees
    Ballistic ball("ball", 90.0, 90.0);
    std::vector<float> first = ball.firstWaypoint(dest3[0], dest3[1], dest3[2]);
    std::vector<float> second = ball.secondWaypoint(dest3[0], dest3[1], dest3[2]);

    BOOST_CHECK(size(first) == 2);

    if (size(first) == 2)
    {
        BOOST_CHECK(fabs(first[0] - 0.0) < 0.001);
        BOOST_CHECK(fabs(first[1] - 0.0) < 0.001);
    }

    BOOST_CHECK(size(second) == 2);

    if (size(second) == 2)
    {
        BOOST_CHECK(fabs(second[0] - 30.0) < 0.001);
        BOOST_CHECK(fabs(second[1] - 30.0) < 0.001);
    }
}

BOOST_AUTO_TEST_CASE(type_checks)
{
    BOOST_CHECK(std::is_class_v<Ufo>);
    BOOST_CHECK(std::is_class_v<Ballistic>);
    BOOST_CHECK(std::is_class_v<Vertical>);
    BOOST_CHECK(std::is_abstract_v<Ufo>);
    BOOST_CHECK(std::is_polymorphic_v<Ufo>);
}

BOOST_AUTO_TEST_CASE(ballistic_after_one_flight)
{
    Ballistic ball("ball", 40.0, 60.0);
    ball.flyToDest(dest4[0], dest4[1], dest4[2], 5);
    BOOST_CHECK(size(ball.getPosition()) == 3);

    if (size(ball.getPosition()) == 3)
    {
        BOOST_CHECK(fabs(ball.getPosition()[0] - dest4[0]) < 0.2);
        BOOST_CHECK(fabs(ball.getPosition()[1] - dest4[1]) < 0.2);
        BOOST_CHECK(ball.getPosition()[2] == 0.0);
    }

    BOOST_CHECK(fabs(ball.getFtime() - 20.8) < 3);

    // the waypoints should be independent from the order of the calls
    // of firstWaypoint and secondWaypoint
    std::vector<float> second = ball.secondWaypoint(dest1[0], dest1[1], dest1[2]);
    std::vector<float> first = ball.firstWaypoint(dest1[0], dest1[1], dest1[2]);

    BOOST_CHECK(size(first) == 2);

    if (size(first) == 2)
    {
        BOOST_CHECK(fabs(first[0] - (-1.47772)) < 0.2);
        BOOST_CHECK(fabs(first[1] - 4.26277) < 0.2);
    }

    BOOST_CHECK(size(second) == 2);

    if (size(second) == 2)
    {
        BOOST_CHECK(fabs(second[0] - 5.86826) < 0.1);
        BOOST_CHECK(fabs(second[1] - 7.93552) < 0.1);
    }

    first = ball.firstWaypoint(dest6[0], dest6[1], dest6[2]);
    second = ball.secondWaypoint(dest6[0], dest6[1], dest6[2]);

    BOOST_CHECK(size(first) == 2);

    if (size(first) == 2)
    {
        BOOST_CHECK(fabs(first[0] - (-11.9881)) < 0.2);
        BOOST_CHECK(fabs(first[1] - (-2.97532)) < 0.2);
    }

    BOOST_CHECK(size(second) == 2);

    if (size(second) == 2)
    {
        BOOST_CHECK(fabs(second[0] - (-19.0396)) < 0.1);
        BOOST_CHECK(fabs(second[1] - (-13.5586)) < 0.1);
    }
}

BOOST_AUTO_TEST_CASE(ballistic_is_not_vertical)
{
    // ballistic flight is different from vertical flight
    Ballistic ball("ball", 45.0, 45.0);
    ball.flyToDest(8.0, 0.0, 4.0, 5);
    BOOST_CHECK(size(ball.getPosition()) == 3);
    BOOST_CHECK(fabs(ball.getFtime() - 19.3) < 2);
}

BOOST_AUTO_TEST_SUITE_END()