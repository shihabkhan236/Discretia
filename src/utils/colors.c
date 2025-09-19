#include "colors.h"
#include <math.h>

Color LerpColor(Color start, Color end, float t) {
    if (t < 0.0f) t = 0.0f;
    if (t > 1.0f) t = 1.0f;
    
    return (Color){
        (unsigned char)(start.r + t * (end.r - start.r)),
        (unsigned char)(start.g + t * (end.g - start.g)),
        (unsigned char)(start.b + t * (end.b - start.b)),
        (unsigned char)(start.a + t * (end.a - start.a))
    };
}

Color FadeColor(Color color, float alpha) {
    if (alpha < 0.0f) alpha = 0.0f;
    if (alpha > 1.0f) alpha = 1.0f;
    
    return (Color){
        color.r,
        color.g,
        color.b,
        (unsigned char)(255 * alpha)
    };
}