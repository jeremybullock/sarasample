#include "stroke.h"
#include "stopmotion/stopmotion.h"



Stroke::Stroke() {
}

Stroke::Stroke(const Stroke& stroke) {
    m_bezierPoints = stroke.m_bezierPoints;
    m_steps = stroke.m_steps;
    m_color = stroke.m_color;
    m_thickness = stroke.m_thickness;
}

Stroke::~Stroke() {};

void Stroke::setPoints(std::vector<QPointF> points) {
    m_bezierPoints = points;
}

