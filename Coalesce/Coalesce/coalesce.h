/*****************************************************************//**
 * \file   coalesce.h
 * \brief  It contains the function: 
 *         coalesce(T&& default_value, Args&&... to_test_v).
 *         It looks for a not null value in to_test_v. If it does not find it, 
 *         then coalesce returns default_value. It is similar to a SQL 
 *         coalesce function.
 * 
 * \author Roberto
 * \date   July 2021
 *********************************************************************/

#ifndef __COALESCE_H
#define __COALESCE_H

#include <utility>
#include <concepts>
#include <memory>
#include <functional>


namespace s4 // Small Simple Stupid Stuff namespace 
{

    /**
     * Concept that define a pointer of type PointerType that points to
     * ValueType. ValueType can be a function (an invocable).
     * It covers: raw pointers, std::unique_ptr, std:shared_ptr,
     * std::weak_ptr, std::optional, std::null_ptr, and any other
     * class that has the operators * and bool.
     */
    template<typename PointerType, typename ValueType>
    concept pointer_to = 
    (    std::invocable<ValueType> 
         &&
         ( 
            requires(PointerType pointer, ValueType value)  // Pointer must behave like a raw pointer to an object of class decltype(value())
            {
                {!pointer};
                {*pointer} -> std::convertible_to<decltype(value())>;
            }
            || requires(PointerType pointer, ValueType value) // Pointer must behave like a weak_ptr that points to an object of class decltype(value())
            {
                {*pointer.lock()} -> std::convertible_to<decltype(value())>; 
            }
         )
    )
    || 
    (   !std::invocable<ValueType>
        && 
        (
            requires(PointerType pointer, ValueType)  // Pointer must behave like a raw pointer that points to a ValueType
            {
                {!pointer};
                {*pointer} -> std::convertible_to<ValueType>;
            }
            || requires(PointerType pointer, ValueType) // Pointer must behave like a weak_ptr that points to a ValueType
            {
                {*pointer.lock()} -> std::convertible_to<ValueType>;
            }
        )
    )
    || std::same_as<std::nullptr_t, PointerType>; // to cover the case of null_ptr


    /**
     * Concept that define the requirements of the coalesce parameters, they can 
     * be pointers or functions that return pointers.
     */
    template<typename PointerType, typename ValueType>
    concept coalesce_param =
    requires(PointerType callable, ValueType)
    {
        {callable()} -> pointer_to<ValueType>;
    }
    || pointer_to<PointerType, ValueType>;


    namespace coalesce_impl
    {
        template<typename ValueType, typename PointerType>
        concept raw_pointer_to = requires(ValueType, PointerType pointer)
        {
            {*pointer} -> std::convertible_to<ValueType>;
            {!pointer};
        }
        || requires(ValueType value, PointerType pointer)
        {
            {*pointer} -> std::convertible_to<decltype(value())>;
            {!pointer};
        };

        template<typename ValueType, typename PointerType>
        concept weak_pointer_to = requires(PointerType pointer, ValueType)
        {
            {*pointer.lock()} noexcept -> std::convertible_to<ValueType>;
        };

        template<typename ValueType, typename PointerType>
        concept callable_ptr_to = requires(PointerType callable, ValueType)
        {
            {callable()}  -> pointer_to<ValueType>;
        };


        /**
         * It looks for a not null value in to_test_0 and to_test_v. If it does
         * not find it, then coalesce returns a default value. It is similar to
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
         *          then coalesce returns default_value.
         */
        template<typename RT, typename DT, typename PT, typename... Args>
        requires raw_pointer_to<RT, PT>
        constexpr decltype(auto) coalesce(DT&& default_value, PT&& to_test_0, Args&&... to_test_v);

        /**
         * Specialized version of coalesce for weak_ptr.
         *
         * \param default_value Value to return if to_test_0 and all to_test_v are null
         * \param to_test_0 First value to check
         * \param ...to_test_v Next values to check. They will be passed in the next iteration of the recursion
         * \return If to_test_0 is not null then it returns it, otherwise it looks for the first element of 
         *         to_test_v not null. If all the values are null then coalesce returns default_value.
         */
        template<typename RT, typename DT, typename PT, typename... Args>
        requires weak_pointer_to<RT, PT> 
        constexpr RT coalesce(DT&& default_value, PT&& to_test_0, Args&&... to_test_v);
        


        /**
         * Specialized version of coalesce for shared_ptr.
         * The shared_ptr parameter is passed by value in order to increment the reference counter.
         *
         * \param default_value Value to return if to_test_0 and all to_test_v are null
         * \param to_test_0 First value to check
         * \param ...to_test_v Next values to check. They will be passed in the next iteration of the recursion
         * \return If to_test_0 is not null then it returns it, otherwise it looks for the first 
         *         element of to_test_v not null. If all the values are null then coalesce returns default_value.
         */
        template<typename RT, typename DT, typename PT, typename... Args>
        requires std::convertible_to< PT, std::remove_reference_t<RT> > 
        constexpr decltype(auto) coalesce(DT&& default_value, std::shared_ptr<PT> to_test_0, Args&&... to_test_v);


        /**
         * Specialized version of coalesce for callable.
         *
         * \param default_value Value to return if to_test_0 and all to_test_v are null
         * \param to_test_0 First value to check
         * \param ...to_test_v Next values to check. They will be passed in the next iteration of the recursion
         * \return If to_test_0 is not null then it returns it, otherwise it looks for the first
         *         element of to_test_v not null. If all the values are null then coalesce returns default_value.
         */
        template<typename RT, typename DT, typename PT, typename... Args>
        requires callable_ptr_to<RT, PT> 
        constexpr decltype(auto) coalesce(DT&& default_value, PT&& to_test_0, Args&&... to_test_v);


        /**
         * Specialized version of coalesce for nullptr_t.
         *
         * \param default_value Value to return if to_test_0 and all to_test_v are null
         * \param ...to_test_v Next values to check. They will be passed in the next iteration of the recursion
         * \return It looks for the first element of to_test_v not null. If all the values are null 
         *         then coalesce returns default_value.
         */
        template<typename RT, typename DT, typename PT, typename... Args>
        constexpr decltype(auto) coalesce(DT&& default_value, std::nullptr_t, Args&&... to_test_v);


        /**
        * It just return the default_value, this function is used to end the recursion of coalesce.
        *
        * \param default_value The value to return
        * \return default_value
        */
        template<typename RT, typename DT>
        requires std::convertible_to< DT, std::remove_reference_t<RT> > 
            && (!std::invocable<DT>)
        constexpr decltype(auto) coalesce(DT&& default_value)
        {
            return static_cast<RT>(default_value);
        }

        
        template<typename RT, typename DT>
        requires std::convertible_to< std::invoke_result_t<DT>, std::remove_reference_t<RT> >
            && std::invocable<DT>
        constexpr decltype(auto) coalesce(DT&& default_value)
        {
            return default_value();
        }

        template<typename RT, typename DT, typename PT, typename... Args>
        requires raw_pointer_to<RT, PT> 
        constexpr decltype(auto) coalesce(DT&& default_value, PT&& to_test_0, Args&&... to_test_v)
        {
            return  
                !to_test_0 
                    ? coalesce<RT, DT, Args...>(
                        std::forward<DT>(default_value),
                        std::forward<Args>(to_test_v)...)
                    : static_cast<RT>(*to_test_0);
        }


        template<typename RT, typename DT, typename PT, typename... Args>
        requires weak_pointer_to<RT, PT> 
        constexpr RT coalesce(DT&& default_value, PT&& to_test_0, Args&&... to_test_v)
        {
            if (auto t = to_test_0.lock())
                return *t;
            else
                return coalesce<RT, DT, Args...>(
                    std::forward<DT>(default_value),
                    std::forward<Args>(to_test_v)...);
        }


        template<typename RT, typename DT, typename PT, typename... Args>
        requires std::convertible_to< PT, std::remove_reference_t<RT> > 
        constexpr decltype(auto) coalesce(DT&& default_value, std::shared_ptr<PT> to_test_0, Args&&... to_test_v)
        {
            return !to_test_0 
                ? coalesce<RT, DT, Args...>(
                    std::forward<DT>(default_value),
                    std::forward<Args>(to_test_v)...)
                : static_cast<RT>(*to_test_0);
        }

        
        template<typename... T>
        struct is_std_function : std::false_type {};

        template<typename... T>
        struct is_std_function<std::function<T...>> : std::true_type {};


        template<typename RT, typename DT, typename PT, typename... Args>
        requires callable_ptr_to<RT, PT> 
        constexpr decltype(auto) coalesce(DT&& default_value, PT&& to_test_0, Args&&... to_test_v)
        {
            if constexpr (is_std_function< std::decay_t<PT> >::value)
            {
                if (!to_test_0)
                {
                    return coalesce<RT, DT, Args...>(
                        std::forward<DT>(default_value),
                        std::forward<Args>(to_test_v)...);
                }
            }

            return coalesce<RT, DT, std::invoke_result_t<PT>, Args...>(
                std::forward<DT>(default_value),
                std::forward<std::invoke_result_t<PT>>(to_test_0()),
                std::forward<Args>(to_test_v)...);
        }
        


        template<typename RT, typename DT, typename PT, typename... Args>
        constexpr decltype(auto) coalesce(DT&& default_value, std::nullptr_t, Args&&... to_test_v)
        {
            return coalesce<RT, DT, Args...>(
                    std::forward<DT>(default_value),
                    std::forward<Args>(to_test_v)...);
        }
    }

    /**
    * It looks for a not null value in to_test_v. If it does
    * not find it, then coalesce returns default_value. It is similar to
    * a SQL coalesce function.
      *
    * \param default_value Value to return if to_test_0 and all to_test_v are null
    * \param ...to_test_v Next values to check. 
    * \return  It looks for the first element of to_test_v not null. 
    *           If all the values are null then coalesce returns default_value.
    */
    template<typename ReturnType, typename DefaultType, coalesce_param<ReturnType>... Args>
    requires std::convertible_to< DefaultType, std::remove_reference_t<ReturnType> >
    constexpr decltype(auto) coalesce(DefaultType&& default_value, Args&&... to_test_v)
    {
        return s4::coalesce_impl::coalesce<ReturnType, DefaultType, Args...>(
            std::forward<DefaultType>(default_value),
            std::forward<Args>(to_test_v)...);
    }

    /**
     * Specialized version of coalesce: the retrun type is a reference of 
     * DefaultType and DefaultType is not invocable.
     */
    template<typename DefaultType, coalesce_param<DefaultType>... Args>
    requires (!std::invocable<DefaultType>)
    constexpr decltype(auto) coalesce(DefaultType&& default_value, Args&&... to_test_v)
    {
        return s4::coalesce_impl::coalesce<DefaultType, DefaultType, Args...>(
            std::forward<DefaultType>(default_value),
            std::forward<Args>(to_test_v)...);
    }

    /**
     * Specialized version of coalesce: DefaultType is invocable, the return type of 
     * coalesce is the same return type of the function passed as default parameter.
     */
    template<typename DefaultType, coalesce_param<DefaultType>... Args>
    requires std::invocable<DefaultType>
    constexpr std::invoke_result_t<DefaultType> coalesce(DefaultType&& default_value, Args&&... to_test_v)
    {
        return s4::coalesce_impl::coalesce<std::invoke_result_t<DefaultType>, DefaultType, Args...>(
                std::forward<DefaultType>(default_value),
                std::forward<Args>(to_test_v)...) ;
    }
    
} // end namespace s4

#endif
