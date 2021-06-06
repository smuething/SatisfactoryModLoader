#pragma once

#include <type_traits>

template<typename T>
struct is_enum_bitfield : std::false_type {};

template<typename T>
static constexpr bool is_enum_bitfield_v = is_enum_bitfield<T>::value;

template<typename Enum>
constexpr std::enable_if_t<is_enum_bitfield_v<Enum>,std::underlying_type_t<Enum>> Flag(Enum flag)
{
    return static_cast<std::underlying_type_t<Enum>>(1) << static_cast<std::underlying_type_t<Enum>>(flag);
}

template<typename Enum>
constexpr std::enable_if_t<is_enum_bitfield_v<Enum>,bool> IsSet(std::underlying_type_t<Enum> BitField, Enum flag)
{
    return BitField & Flag(flag);
}

template<typename Enum>
constexpr std::enable_if_t<is_enum_bitfield_v<Enum>,std::underlying_type_t<Enum>> SetFlag(std::underlying_type_t<Enum> BitField, Enum flag)
{
    return BitField | Flag(flag);
}

template<typename Enum>
constexpr std::enable_if_t<is_enum_bitfield_v<Enum>,std::underlying_type_t<Enum>> ClearFlag(std::underlying_type_t<Enum> BitField, Enum flag)
{
    return BitField & ~Flag(flag);
}

template<typename Enum>
constexpr std::enable_if_t<is_enum_bitfield_v<Enum>,std::underlying_type_t<Enum>> SetFlag(std::underlying_type_t<Enum> BitField, Enum flag, bool Enabled)
{
    return (BitField & ~Flag(flag)) | (Enabled * Flag(flag));
}



constexpr static int32 Pow_Constexpr(int32 Base, int32 Exponent)
{
    int32 Result = 1;
    while (Exponent-- > 0)
        Result *= Base;
    return Result;
}
