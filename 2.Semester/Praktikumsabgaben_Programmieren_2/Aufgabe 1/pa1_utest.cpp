#define BOOST_TEST_MAIN
#include <cmath>
#include <type_traits>
#include <vector>
#include <boost/test/included/unit_test.hpp>
#include "vertical.h"

BOOST_AUTO_TEST_SUITE(pa_utest)

std::vector<float> dest1 = { 10.0, 10.0, 8.0 };
std::vector<float> dest2 = { 10.0, 0.0, 10.0 };

BOOST_AUTO_TEST_CASE(initially)
{
    Vertical vert("r2d2");
    BOOST_CHECK(Vertical::getType() == "vertical");
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

BOOST_AUTO_TEST_CASE(after_one_flight)
{
    Vertical vert("r2d2");
    vert.flyToDest(dest1[0], dest1[1], dest1[2], 10);
    BOOST_CHECK(Vertical::getType() == "vertical");
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

BOOST_AUTO_TEST_CASE(after_two_flights)
{
    Vertical vert("r2d2");
    vert.flyToDest(dest1[0], dest1[1], dest1[2], 10);
    vert.flyToDest(dest2[0], dest2[1], dest2[2], 15);
    BOOST_CHECK(Vertical::getType() == "vertical");
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
    std::vector<float> B = Vertical::wayPoint(x1, y1, x2, y2, h, phi);
    return fabs((B[0] - x1) * (y2 - y1) - (B[1] - y1) * (x2 - x1) < 0.001);
}

BOOST_AUTO_TEST_CASE(waypoint)
{
    // basic tests
    std::vector<float> B = Vertical::wayPoint(2.0, 0.0, 100.3f, 0.0, 10.0, 45.0);
    BOOST_CHECK(fabs(B[0] - 12.0) < 0.001);
    BOOST_CHECK(fabs(B[1] - 0.0) < 0.001);
    B = Vertical::wayPoint(0.0, -20.0, 0.0, 55.5, 10.0, 45.0);
    BOOST_CHECK(fabs(B[0] - 0.0) < 0.001);
    BOOST_CHECK(fabs(B[1] - -10.0) < 0.001);

    // legacy tests
    B = Vertical::wayPoint(0.0, 0.0, 30.0, 30.0, 8.0, 45.0);
    BOOST_CHECK(fabs(B[0] - 5.65685) < 0.001);
    BOOST_CHECK(fabs(B[1] - 5.65685) < 0.001);
    // 1st quadrant to 1st quadrant, north east, near A (0.188562)
    BOOST_CHECK(check_waypoint(0.0, 0.0, 30.0, 30.0, 8.0, 45.0));

    B = Vertical::wayPoint(0.0, 0.0, 10.0, 20.0, 8.0, 45.0);
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

BOOST_AUTO_TEST_CASE(type_checks)
{
    BOOST_CHECK(std::is_class_v<Vertical>);
}

BOOST_AUTO_TEST_SUITE_END()
