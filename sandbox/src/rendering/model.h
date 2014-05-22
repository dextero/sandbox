#ifndef MODEL_H
#define MODEL_H

#include "drawable.h"
#include <string>

namespace sb
{
    class Model: public Drawable
    {
    public:
        Model();
        Model(const std::string& path);

        bool loadFromFile(const std::string& path);
    };
} // namespace sb

#endif //MODEL_H
