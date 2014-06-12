#ifndef MODEL_H
#define MODEL_H

#include "drawable.h"
#include <string>

namespace sb
{
    class Model: public Drawable
    {
    public:
        Model(const std::string& path,
              const std::shared_ptr<Shader>& shader);

        bool loadFromFile(const std::string& path);
    };
} // namespace sb

#endif //MODEL_H
