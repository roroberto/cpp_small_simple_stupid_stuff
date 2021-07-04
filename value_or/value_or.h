/*****************************************************************//**
 * \file   value_or.h
 * \brief  It contains the function: 
 *         value_or(T&& default_value, Args&&... to_test_v).
 *         It looks for a not null value in to_test_v. If it does not find it, 
 *         then value_or returns default_value. It is similar to a SQL 
 *         coalesce function.
 * 
 * \author Roberto
 * \date   July 2021
 *********************************************************************/

#ifndef __VALUE_OR_H
#define __VALUE_OR_H

#include <utility>
#include <concepts>
#include <memory>


namespace s4 // Small Simple Stupid Stuff namespace 
{

    /**
     * Concept that define a pointer of type PointerType that points to 
     * ValueType.
     * It covers: raw pointers, std::unique_ptr, std:shared_ptr, 
     * std::weak_ptr, std::optional, std::null_ptr, NULL, 0 and any other 
     * class that has the operators * and bool.
     */
    template<typename PointerType, typename ValueType>
    concept PointerTo = requires(PointerType pointer, ValueType)  // Pointer must behave like a raw pointer that points to a ValueType
    {
        {*pointer} -> std::convertible_to<ValueType>;
        {!pointer} noexcept;
    }
    || requires(PointerType pointer, ValueType) // Pointer must behave like a weak_ptr that points to a ValueType
    {
        {*pointer.lock()} noexcept -> std::convertible_to<ValueType>;
    }
    || std::same_as<std::nullptr_t, PointerType> // to cover the case of null_ptr
    || std::same_as<int, PointerType>;  // for NULL (in VS) and 0


    /**
    * It looks for a not null value in to_test_v. If it does not find it, 
    * then value_or returns a default value. 
    *
    * \param default_value Value to return if all the values of to_test_v are null
    * \param ...to_test_v The values to test for not null
    * \return The first value not null of to_test_v, if no value of to_test_v is not null then default_value
    */
    template<typename T, PointerTo<T>... Args>
    constexpr decltype(auto) value_or(T&& default_value, Args&&... to_test_v) noexcept;
    



    namespace value_or_impl
    {
        template<typename T, typename PT>
        concept value_or_req = requires(T x, PT y)
        {
            {*y} -> std::convertible_to<T>;
            {!y} noexcept;
        };


        /**
         * It looks for a not null value in to_test_0 and to_test_v. If it does
         * not find it, then value_or returns a default value. It is similar to
         * a SQL coalesce function.
         * It is a recursive function. There are specialized versions to fit 
         * better the different needs. The recursion ends when a not null pointer 
         * is found or when only the default value is left.
         *
         * \param default_value Value to return if to_test_0 and all to_test_v are null
         * \param to_test_0 First value to check
         * \param ...to_test_v Next values to check. They will be passed in the next iteration of the recursion
         * \return  If to_test_0 is not null then it returns it, otherwise it looks 
         *          for the first element of to_test_v not null. If all the values are null 
         *          then value_or returns default_value.
         */
        template<typename T, typename PT, typename... Args>
        requires value_or_req<T, PT>
        constexpr decltype(auto) value_or(T&& default_value, PT&& to_test_0, Args&&... to_test_v) noexcept;

        /**
         * Specialized version of value_or for weak_ptr.
         *
         * \param default_value Value to return if to_test_0 and all to_test_v are null
         * \param to_test_0 First value to check
         * \param ...to_test_v Next values to check. They will be passed in the next iteration of the recursion
         * \return If to_test_0 is not null then it returns it, otherwise it looks for the first element of 
         *         to_test_v not null. If all the values are null then value_or returns default_value.
         */
        template<typename T, typename PT, typename... Args>
        requires std::convertible_to< PT, std::remove_reference_t<T> >
        constexpr decltype(auto) value_or(T&& default_value, std::weak_ptr<PT>& to_test_0, Args&&... to_test_v) noexcept;

        /**
         * Specialized version of value_or for shared_ptr.
         * The shared_ptr parameter is passed by value in order to increment the reference counter.
         *
         * \param default_value Value to return if to_test_0 and all to_test_v are null
         * \param to_test_0 First value to check
         * \param ...to_test_v Next values to check. They will be passed in the next iteration of the recursion
         * \return If to_test_0 is not null then it returns it, otherwise it looks for the first 
         *         element of to_test_v not null. If all the values are null then value_or returns default_value.
         */
        template<typename T, typename PT, typename... Args>
        requires std::convertible_to< PT, std::remove_reference_t<T> >
        constexpr decltype(auto) value_or(T&& default_value, std::shared_ptr<PT> to_test_0, Args&&... to_test_v) noexcept;

        /**
         * Specialized version of value_or for nullptr_t.
         *
         * \param default_value Value to return if to_test_0 and all to_test_v are null
         * \param ...to_test_v Next values to check. They will be passed in the next iteration of the recursion
         * \return It looks for the first element of to_test_v not null. If all the values are null 
         *         then value_or returns default_value.
         */
        template<typename T, typename... Args>
        constexpr decltype(auto) value_or(T&& defaultValue, std::nullptr_t, Args&&... to_test_v) noexcept;

        /**
         * Specialized version of value_or for NULL or 0.
         *
         * \param default_value Value to return if to_test_0 and all to_test_v are null
         * \param ...to_test_v Next values to check. They will be passed in the next iteration of the recursion
         * \return It looks for the first element of to_test_v not null. If all the values are null then 
         *         value_or returns default_value.
         */
        template<typename T, typename... Args>
        constexpr decltype(auto) value_or(T&& defaultValue, int, Args&&... to_test_v) noexcept;



        /**
        * It just return the default_value, this function is used to end the recursion of value_or.
        *
        * \param default_value The value to return
        * \return default_value
        */
        template<typename T>
        constexpr decltype(auto) value_or(T&& default_value) noexcept
        {
            return std::forward<T>(default_value);
        }



        template<typename T, typename PT, typename... Args>
        requires value_or_req<T, PT>
        constexpr decltype(auto) value_or(T&& default_value, PT&& to_test_0, Args&&... to_test_v) noexcept
        {
            return  to_test_0 ? *to_test_0 : std::forward<T>(value_or(default_value, to_test_v...));
        }


        template<typename T, typename PT, typename... Args>
        requires std::convertible_to< PT, std::remove_reference_t<T> >
        constexpr decltype(auto) value_or(T&& default_value, std::weak_ptr<PT>& to_test_0, Args&&... to_test_v) noexcept
        {
            if (auto t = to_test_0.lock())
                return std::forward<T>(*t);
            else
                return std::forward<T>(value_or(default_value, to_test_v...));
        }


        template<typename T, typename PT, typename... Args>
        requires std::convertible_to< PT, std::remove_reference_t<T> >
        constexpr decltype(auto) value_or(T&& default_value, std::shared_ptr<PT> to_test_0, Args&&... to_test_v) noexcept
        {
            return  to_test_0 ? *to_test_0 : std::forward<T>(value_or(default_value, to_test_v...));
        }


        template<typename T, typename... Args>
        constexpr decltype(auto) value_or(T&& default_value, std::nullptr_t, Args&&... to_test_v) noexcept
        {
            return std::forward<T>(value_or(default_value, to_test_v...));
        }

        template<typename T, typename... Args>
        constexpr decltype(auto) value_or(T&& defaultValue, int, Args&&... to_test_v) noexcept
        {
            return std::forward<T>(value_or(defaultValue, to_test_v...));
        }
    }

     
    template<typename T, PointerTo<T>... Args>
    constexpr decltype(auto) value_or(T&& default_value, Args&&... to_test_v) noexcept
    {
        return std::forward<T>(value_or_impl::value_or(default_value, to_test_v...));
    }

} // end namespace s4

#endif
