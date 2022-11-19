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

#include <utility>
#include <concepts>
#include <memory>
#include <functional>


namespace s4 // Small Simple Stupid Stuff namespace 
{

    /**
     * Concept that defines an object that hold a value of type
     * ValueType. ValueType can be also an invocable, this is 
     * needed if the default value is an invocable.
     * It covers: raw pointers, std::unique_ptr, std:shared_ptr,
     * std::weak_ptr, std::optional, std::nullptr, and any other
     * class that has the operators * and !.
     */
    template<typename ValueHolderType, typename ValueType>
    concept value_or_value_holder = 
    (   std::invocable<ValueType>
        &&
        ( 
            // value_holder must behave like a raw pointer to an object of class 
            // decltype(value())
            requires(ValueHolderType value_holder, ValueType value)  
            {
                {!value_holder};
                {*value_holder} -> std::convertible_to<decltype(value())>;
            }
            // OR value_holder must behave like a weak_ptr that points to an object of 
            // class decltype(value())
            || requires(ValueHolderType value_holder, ValueType value)
            {
                {*value_holder.lock()} -> std::convertible_to<decltype(value())>;
            }
        )
    )
    || 
    (   !std::invocable<ValueType>
        && 
        (
            // value_holder must behave like a raw pointer that points to a ValueType
            requires(ValueHolderType value_holder, ValueType)
            {
                {!value_holder};
                {*value_holder} -> std::convertible_to<ValueType>;
            }
            // OR value_holder must behave like a weak_ptr that points to a ValueType
            || requires(ValueHolderType value_holder, ValueType)
            {
                {*value_holder.lock()} -> std::convertible_to<ValueType>;
            }
        )
    )
    // to cover the case of null_ptr
    || std::same_as<std::nullptr_t, ValueHolderType>; 


    /**
     * Concept that defines the requirements of the value_or parameters, 
     * they can be pointers or functions that return pointers.
     */
    template<typename ValueHolderType, typename ValueType>
    concept value_or_param =
    requires(ValueHolderType callable, ValueType)
    {
        {callable()} -> value_or_value_holder<ValueType>;
    }
    || value_or_value_holder<ValueHolderType, ValueType>;


    namespace value_or_impl
    {
        template<typename ValueType, typename ValueHolderType>
        concept raw_pointer_to = requires(ValueType, ValueHolderType pointer)
        {
            {*pointer} -> std::convertible_to<ValueType>;
            {!pointer};
        }
        || requires(ValueType value, ValueHolderType pointer)
        {
            {*pointer} -> std::convertible_to<decltype(value())>;
            {!pointer};
        };

        template<typename ValueType, typename ValueHolderType>
        concept weak_pointer_to = requires(ValueHolderType pointer, ValueType)
        {
            {*pointer.lock()} noexcept -> std::convertible_to<ValueType>;
        };

        template<typename ValueType, typename ValueHolderType>
        concept callable_ptr_to = requires(ValueHolderType callable, ValueType)
        {
            {callable()}  -> value_or_value_holder<ValueType>;
        };


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
        requires raw_pointer_to<RT, PT>
        constexpr decltype(auto) value_or(DT&& default_value, PT&& to_test_0, Args&&... to_test_v);

        /**
         * Specialized version of value_or for weak_ptr.
         *
         * \param default_value Value to return if to_test_0 and all to_test_v are null
         * \param to_test_0 First value to check
         * \param ...to_test_v Next values to check. They will be passed in the next iteration of the recursion
         * \return If to_test_0 is not null then it returns it, otherwise it looks for the first element of 
         *         to_test_v not null. If all the values are null then value_or returns default_value.
         */
        template<typename RT, typename DT, typename PT, typename... Args>
        requires weak_pointer_to<RT, PT> 
        constexpr std::remove_reference_t<RT> value_or(DT&& default_value, PT&& to_test_0, Args&&... to_test_v);


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
         * Specialized version of value_or for callable.
         *
         * \param default_value Value to return if to_test_0 and all to_test_v are null
         * \param to_test_0 First value to check
         * \param ...to_test_v Next values to check. They will be passed in the next iteration of the recursion
         * \return If to_test_0 is not null then it returns it, otherwise it looks for the first
         *         element of to_test_v not null. If all the values are null then value_or returns default_value.
         */
        template<typename RT, typename DT, typename PT, typename... Args>
        requires callable_ptr_to<RT, PT> 
        constexpr decltype(auto) value_or(DT&& default_value, PT&& to_test_0, Args&&... to_test_v);

        /**
         * Specialized version of value_or for callable where it is possible to test if the pointer to the 
         * callable has a callable
         *
         * \param default_value Value to return if to_test_0 and all to_test_v are null
         * \param to_test_0 First value to check
         * \param ...to_test_v Next values to check. They will be passed in the next iteration of the recursion
         * \return If to_test_0 is not null then it returns it, otherwise it looks for the first
         *         element of to_test_v not null. If all the values are null then value_or returns default_value.
         */
        template<typename RT, typename DT, typename PT, typename... Args>
        requires callable_ptr_to<RT, PT>
        && requires (PT f) { {!f}; }
        constexpr decltype(auto) value_or(DT&& default_value, PT&& to_test_0, Args&&... to_test_v);

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
            && (!std::invocable<DT>)
        constexpr decltype(auto) value_or(DT&& default_value)
        {
            return static_cast<RT>(default_value);
        }

        /**
        * It returns default_value() in the case the default value is invocable. 
        * this function is used to end the recursion of value_or.
        *
        * \param default_value The value to return
        * \return default_value
        */
        template<typename RT, typename DT>
        requires std::convertible_to< std::invoke_result_t<DT>, std::remove_reference_t<RT> >
            && std::invocable<DT>
        constexpr decltype(auto) value_or(DT&& default_value)
        {
            return default_value();
        }

                  

        template<typename RT, typename DT, typename PT, typename... Args>
        requires raw_pointer_to<RT, PT> 
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
        requires weak_pointer_to<RT, PT> 
        constexpr std::remove_reference_t<RT> value_or(DT&& default_value, PT&& to_test_0, Args&&... to_test_v)
        {
            if (auto t = to_test_0.lock())
                return *t;
            else
                return value_or<RT, DT, Args...>(
                    std::forward<DT>(default_value),
                    std::forward<Args>(to_test_v)...);
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
        requires callable_ptr_to<RT, PT>
        constexpr decltype(auto) value_or(DT&& default_value, PT&& to_test_0, Args&&... to_test_v)
        {
            return value_or<RT, DT, std::invoke_result_t<PT>, Args...>(
                std::forward<DT>(default_value),
                std::forward<std::invoke_result_t<PT>>(to_test_0()),
                std::forward<Args>(to_test_v)...);
        }
        

        template<typename RT, typename DT, typename PT, typename... Args>
        requires callable_ptr_to<RT, PT>  
        && requires (PT f) { {!f}; }
        constexpr decltype(auto) value_or(DT&& default_value, PT&& to_test_0, Args&&... to_test_v)
        {
            if (!to_test_0)
            {
                return value_or<RT, DT, Args...>(
                    std::forward<DT>(default_value),
                    std::forward<Args>(to_test_v)...);
            }
            else
            {
                return value_or<RT, DT, std::invoke_result_t<PT>, Args...>(
                    std::forward<DT>(default_value),
                    std::forward<std::invoke_result_t<PT>>(to_test_0()),
                    std::forward<Args>(to_test_v)...);
            }
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
    template<typename ReturnType, typename DefaultType, value_or_param<ReturnType>... Args>
    requires std::convertible_to< DefaultType, std::remove_reference_t<ReturnType> >
    constexpr ReturnType value_or(DefaultType&& default_value, Args&&... to_test_v)
    {
        return s4::value_or_impl::value_or<ReturnType, DefaultType, Args...>(
            std::forward<DefaultType>(default_value),
            std::forward<Args>(to_test_v)...);
    }

    /**
     * Specialized version of value_or: the return type is a reference of 
     * DefaultType and DefaultType is not invocable.
     */
    template<typename DefaultType, value_or_param<DefaultType>... Args>
    requires (!std::invocable<DefaultType>)
    constexpr decltype(auto) value_or(DefaultType&& default_value, Args&&... to_test_v)
    {
        return s4::value_or_impl::value_or<DefaultType, DefaultType, Args...>(
            std::forward<DefaultType>(default_value),
            std::forward<Args>(to_test_v)...);
    }

    /**
     * Specialized version of value_or: DefaultType is invocable, the return type of 
     * value_or is the same return type of the function passed as default parameter.
     */
    template<typename DefaultType, value_or_param<DefaultType>... Args>
    requires std::invocable<DefaultType>
    constexpr std::invoke_result_t<DefaultType> value_or(DefaultType&& default_value, Args&&... to_test_v)
    {
        return s4::value_or_impl::value_or<std::invoke_result_t<DefaultType>, DefaultType, Args...>(
                std::forward<DefaultType>(default_value),
                std::forward<Args>(to_test_v)...) ;
    }
  
} // end namespace s4

#endif
