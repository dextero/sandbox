#ifndef UTILS_VECTOR_H
#define UTILS_VECTOR_H

namespace sb
{
    // please don't hate me
#define OP1(RetType, op) \
    inline RetType operator op() const \
    { return glm::detail::operator op((const base_type&)*this); }
#define OP2(RetType, ArgType, op) \
    inline RetType operator op(ArgType arg) \
    { return base_type::operator op(arg); }
#define OP2_T(Template, RetType, ArgType, op) \
    template<typename OpArgType, glm::precision OpPrec> \
    inline RetType operator op(const Template<OpArgType, OpPrec>& arg) \
    { return (RetType)base_type::operator op(arg); }
#define OP2_CONST(RetType, ArgType, op) \
    inline RetType operator op(ArgType arg) const \
    { return RetType(glm::detail::operator op((const base_type&)*this, arg)); }
#define OP2_CONST_T(Template, RetType, ArgType, op) \
    template<typename OpArgType> \
    inline RetType operator op(const Template<OpArgType>& arg) const \
    { return RetType(glm::detail::operator op((const base_type&)*this, arg)); }

#define OPERATORS(BaseType, VecType, ElemType) \
    OP1(VecType, +) \
    OP1(VecType, -) \
    \
    OP2_T(BaseType, VecType&, const VecType&, +=) \
    OP2_T(BaseType, VecType&, const VecType&, -=) \
    OP2(VecType&, ElemType, *=) \
    OP2(VecType&, ElemType, /=) \
    \
    OP2_CONST_T(VecType, VecType, const VecType&, +) \
    OP2_CONST_T(VecType, VecType, const VecType&, -) \
    OP2_CONST(VecType, ElemType, *) \
    OP2_CONST(VecType, ElemType, /) \
    \
    OP2_CONST_T(VecType, bool, const VecType&, ==) \
    OP2_CONST_T(VecType, bool, const VecType&, !=)

    template<typename ElemT>
    struct TVec2: public glm::detail::tvec2<ElemT, glm::highp>
    {
        typedef glm::detail::tvec2<ElemT, glm::highp> base_type;

        TVec2() {}
        TVec2(ElemT x, ElemT y):
            base_type(x, y)
        {}
        TVec2(const TVec2& v):
            base_type((const base_type&)v)
        {}
        TVec2(const base_type& v):
            base_type(v)
        {}

        template<typename T>
        TVec2(const TVec2<T>& v):
            base_type((const typename TVec2<T>::base_type&)v)
        {}

        template<typename T, glm::precision P>
        explicit TVec2(const glm::detail::tvec2<T, P>& v):
            base_type(v)
        {}

        inline float length() const
        {
            return glm::length((const base_type&)*this);
        }

        inline float dot(const base_type& v) const
        {
            return glm::dot((const base_type&)*this, v);
        }

        inline TVec2 normalized() const
        {
            return TVec2(glm::normalize((const base_type&)*this));
        }

        inline bool isZero() const
        {
            return this->x == (ElemT)0 && this->y == (ElemT)0;
        }

        OPERATORS(glm::detail::tvec2, TVec2, ElemT)
    };

    template<typename ElemT>
    struct TVec3: public glm::detail::tvec3<ElemT, glm::highp>
    {
        typedef glm::detail::tvec3<ElemT, glm::highp> base_type;

        TVec3() {}
        TVec3(ElemT x, ElemT y, ElemT z):
            base_type(x, y, z)
        {}
        TVec3(const base_type& v):
            base_type(v)
        {}

        template<typename T>
        TVec3(const TVec3<T>& v):
            base_type((const typename TVec3<T>::base_type&)v)
        {}

        template<typename T, glm::precision P>
        explicit TVec3(const glm::detail::tvec3<T, P>& v):
            base_type(v)
        {}

        inline float length() const
        {
            return glm::length((const base_type&)*this);
        }

        inline float dot(const base_type& v) const
        {
            return glm::dot((const base_type&)*this, v);
        }
        inline TVec3 cross(const base_type& v) const
        {
            return TVec3(glm::cross((const base_type&)*this, v));
        }

        inline TVec3 normalized() const
        {
            return TVec3(glm::normalize((const base_type&)*this));
        }

        inline bool isZero() const
        {
            return this->x == (ElemT)0 && this->y == (ElemT)0 && this->z == (ElemT)0;
        }

        OPERATORS(glm::detail::tvec3, TVec3, ElemT)
    };

    template<typename ElemT>
    struct TVec4: public glm::detail::tvec4<ElemT, glm::highp>
    {
        typedef glm::detail::tvec4<ElemT, glm::highp> base_type;

        TVec4() {}
        TVec4(ElemT x, ElemT y, ElemT z, ElemT w):
            base_type(x, y, z, w)
        {}
        TVec4(const base_type& v):
            base_type(v)
        {}

        template<typename T>
        TVec4(const TVec4<T>& v):
            base_type((const typename TVec4<T>::base_type&)v)
        {}

        template<typename T, glm::precision P>
        explicit TVec4(const glm::detail::tvec4<T, P>& v):
            base_type(v)
        {}

        template<typename T, glm::precision P>
        explicit TVec4(const glm::detail::tvec3<T, P>& v):
            base_type(v.x, v.y, v.z, (T)1)
        {}

        inline float length() const
        {
            return glm::length((const base_type&)*this);
        }

        inline float dot(const base_type& v) const
        {
            return glm::dot((const base_type&)*this, v);
        }

        inline TVec4 normalized() const
        {
            return TVec4(glm::normalize((const base_type&)*this));
        }

        inline bool isZero() const
        {
            return this->x == (ElemT)0
                   && this->y == (ElemT)0
                   && this->z == (ElemT)0
                   && this->w == (ElemT)0;
        }

        OPERATORS(glm::detail::tvec4, TVec4, ElemT)
    };

#undef OPERATORS
#undef OP2_CONST
#undef OP2
#undef OP1

} // namespace sb

#endif /* UTILS_VECTOR_H */
