#pragma once
#include <Arduino.h>

namespace Utils {
    class Interpolation {
    public:
        // Linear interpolation between two points
        static float lerp(float a, float b, float t) {
            return a + (b - a) * constrain(t, 0.0f, 1.0f);
        }

        // Smooth step interpolation (cubic)
        static float smoothStep(float a, float b, float t) {
            t = constrain(t, 0.0f, 1.0f);
            t = t * t * (3.0f - 2.0f * t); // Smooth curve
            return lerp(a, b, t);
        }

        // Exponential easing interpolation
        static float expEase(float a, float b, float t, float exponent = 2.0f) {
            t = constrain(t, 0.0f, 1.0f);
            bool isRising = b > a;
            float power = isRising ? exponent : 1.0f/exponent;
            float easedT = pow(t, power);
            return lerp(a, b, easedT);
        }

        // Bezier curve interpolation
        static float bezier(float p0, float p1, float p2, float p3, float t) {
            t = constrain(t, 0.0f, 1.0f);
            float oneMinusT = 1.0f - t;
            float oneMinusT2 = oneMinusT * oneMinusT;
            float oneMinusT3 = oneMinusT2 * oneMinusT;
            float t2 = t * t;
            float t3 = t2 * t;
            
            return oneMinusT3 * p0 + 
                   3.0f * oneMinusT2 * t * p1 + 
                   3.0f * oneMinusT * t2 * p2 + 
                   t3 * p3;
        }

        // Multi-point linear interpolation
        template<typename T>
        static float multiLerp(const T* points, const float* values, 
                             int count, float x) {
            // Handle edge cases
            if (count < 2) return count == 1 ? values[0] : 0.0f;
            if (x <= static_cast<float>(points[0])) return values[0];
            if (x >= static_cast<float>(points[count-1])) return values[count-1];

            // Find surrounding points
            int i = 0;
            while (i < count - 1 && static_cast<float>(points[i+1]) < x) i++;

            // Interpolate between found points
            float t = (x - static_cast<float>(points[i])) / 
                     (static_cast<float>(points[i+1]) - static_cast<float>(points[i]));
            return lerp(values[i], values[i+1], t);
        }

        // Smoothing filter for noise reduction
        class SmoothingFilter {
        public:
            SmoothingFilter(float smoothingFactor = 0.1f)
                : alpha(smoothingFactor)
                , lastValue(0.0f)
                , initialized(false) {}

            float update(float newValue) {
                if (!initialized) {
                    lastValue = newValue;
                    initialized = true;
                    return newValue;
                }

                lastValue = lerp(lastValue, newValue, alpha);
                return lastValue;
            }

            void reset() {
                initialized = false;
                lastValue = 0.0f;
            }

            void setSmoothingFactor(float factor) {
                alpha = constrain(factor, 0.0f, 1.0f);
            }

        private:
            float alpha;
            float lastValue;
            bool initialized;
        };

        // Acceleration/deceleration curve generator
        static float accelerationCurve(float progress, float acceleration = 2.0f) {
            progress = constrain(progress, 0.0f, 1.0f);
            if (progress < 0.5f) {
                // Acceleration phase
                return 0.5f * pow(2.0f * progress, acceleration);
            } else {
                // Deceleration phase
                return 1.0f - 0.5f * pow(2.0f * (1.0f - progress), acceleration);
            }
        }

        // Spring-based smoothing for natural motion
        class SpringSmoothing {
        public:
            SpringSmoothing(float springConstant = 10.0f, float damping = 0.8f)
                : k(springConstant)
                , d(damping)
                , position(0.0f)
                , velocity(0.0f) {}

            float update(float target, float deltaTime) {
                float force = (target - position) * k;
                velocity = velocity + force * deltaTime;
                velocity = velocity * (1.0f - d * deltaTime);
                position = position + velocity * deltaTime;
                return position;
            }

            void reset(float initialPosition = 0.0f) {
                position = initialPosition;
                velocity = 0.0f;
            }

        private:
            float k;          // Spring constant
            float d;          // Damping factor
            float position;   // Current position
            float velocity;   // Current velocity
        };
    };
}
