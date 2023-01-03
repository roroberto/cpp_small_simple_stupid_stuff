/**********************************************************************
 * \file   value_or.h
 * \brief  It contains the function: 
 *         value_or(T&& default_value, Args&&... to_test_v).
 *         It looks for an argument with value in to_test_v. 
 *         If value_or does not find an argument with a value 
 *         then it returns default_value. It is similar to the SQL
 *         function coalesce. It is also similar to the method value_or 
 *         of std::optional. 
 * 
 * \author Roberto
 * \date   July 2022
 *********************************************************************/

#ifndef __value_or_H
#define __value_or_H

#include <concepts>
#include <memory>


namespace s4 // Small Simple Stupid Stuff namespace 
{

    /**
     * Concept that defines an object that hold a value of type
     * ValueType. 
     * It covers: raw pointers, std::unique_ptr, std:shared_ptr,
     * std::optional, std::nullptr, and any other
     * class that has the operators * and !.
     * operator * return an object convertible_to<ValueType>
     */
    template<typename ValueHolderType, typename ValueType>
    concept value_or_value_holder = 
    (    
        requires(ValueHolderType value_holder, ValueType value_type)
        {
            { (!value_holder) ? value_type : *value_holder } -> std::convertible_to<ValueType>;
        }
    )
    // to cover the case of null_ptr
    || std::same_as<std::nullptr_t, ValueHolderType>; 



    namespace value_or_impl
    {
        /**
         * It looks for a not null value in to_test_0 and to_test_v. If it does
         * not find it, then value_or returns a default value. It is similar to
         * a SQL value_or function.
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
        template<typename RT, typename DT, typename PT, typename... Args>
        requires value_or_value_holder<PT, RT>
        constexpr decltype(auto) value_or(DT&& default_value, PT&& to_test_0, Args&&... to_test_v);

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
        template<typename RT, typename DT, typename PT, typename... Args>
        requires std::convertible_to< PT, std::remove_reference_t<RT> > 
        constexpr decltype(auto) value_or(DT&& default_value, std::shared_ptr<PT> to_test_0, Args&&... to_test_v);


        /**
         * Specialized version of value_or for nullptr_t.
         *
         * \param default_value Value to return if to_test_0 and all to_test_v are null
         * \param ...to_test_v Next values to check. They will be passed in the next iteration of the recursion
         * \return It looks for the first element of to_test_v not null. If all the values are null 
         *         then value_or returns default_value.
         */
        template<typename RT, typename DT, typename PT, typename... Args>
        constexpr decltype(auto) value_or(DT&& default_value, std::nullptr_t, Args&&... to_test_v);



        /**
        * It returns the default_value, this function is used to end the recursion of value_or.
        *
        * \param default_value The value to return
        * \return default_value
        */
        template<typename RT, typename DT>
        requires std::convertible_to< DT, std::remove_reference_t<RT> > 
        constexpr decltype(auto) value_or(DT&& default_value)
        {
            return static_cast<RT>(default_value);
        }


        template<typename RT, typename DT, typename PT, typename... Args>
        requires value_or_value_holder<PT, RT>
        constexpr decltype(auto) value_or(DT&& default_value, PT&& to_test_0, Args&&... to_test_v)
        {
            return  
                !to_test_0 
                    ? value_or<RT, DT, Args...>(
                        std::forward<DT>(default_value),
                        std::forward<Args>(to_test_v)...)
                    : static_cast<RT>(*to_test_0);
        }


        template<typename RT, typename DT, typename PT, typename... Args>
        requires std::convertible_to< PT, std::remove_reference_t<RT> > 
        constexpr decltype(auto) value_or(DT&& default_value, std::shared_ptr<PT> to_test_0, Args&&... to_test_v)
        {
            return !to_test_0 
                ? value_or<RT, DT, Args...>(
                    std::forward<DT>(default_value),
                    std::forward<Args>(to_test_v)...)
                : static_cast<RT>(*to_test_0);
        }


        template<typename RT, typename DT, typename PT, typename... Args>
        constexpr decltype(auto) value_or(DT&& default_value, std::nullptr_t, Args&&... to_test_v)
        {
            return value_or<RT, DT, Args...>(
                    std::forward<DT>(default_value),
                    std::forward<Args>(to_test_v)...);
        }

    }

    /**
    * It looks for a not null value in to_test_v. If it does
    * not find it, then value_or returns default_value. It is similar to
    * a SQL value_or function.
      *
    * \param default_value Value to return if to_test_0 and all to_test_v are null
    * \param ...to_test_v Next values to check. 
    * \return  It looks for the first element of to_test_v not null. 
    *           If all the values are null then value_or returns default_value.
    */
    template<typename ReturnType, typename DefaultType, value_or_value_holder<ReturnType>... Args>
    requires std::convertible_to< DefaultType, std::remove_reference_t<ReturnType> >
    constexpr ReturnType value_or(DefaultType&& default_value, Args&&... to_test_v)
    {
        return s4::value_or_impl::value_or<ReturnType, DefaultType, Args...>(
            std::forward<DefaultType>(default_value),
            std::forward<Args>(to_test_v)...);
    }

    /**
     * Specialized version of value_or: the return type is a reference of 
     * DefaultType 
     */
    template<typename DefaultType, value_or_value_holder<DefaultType>... Args>
    constexpr decltype(auto) value_or(DefaultType&& default_value, Args&&... to_test_v)
    {
        return s4::value_or_impl::value_or<DefaultType, DefaultType, Args...>(
            std::forward<DefaultType>(default_value),
            std::forward<Args>(to_test_v)...);
    }

  
} // end namespace s4

#endif
