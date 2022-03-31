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



    template<typename PointerType, typename ValueType>
    concept coalesce_param =
    requires(PointerType callable, ValueType)
    {
        {callable()} -> pointer_to<ValueType>;
    }
    || pointer_to<PointerType, ValueType>;


    /**
    * It looks for a not null value in to_test_v. If it does not find it, 
    * then coalesce returns a default value. 
    *
    * \param default_value Value to return if all the values of to_test_v are null
    * \param ...to_test_v The values to test for not null
    * \return The first value not null of to_test_v, if no value of to_test_v is not null then default_value
    */
    //template<typename T, typename DT, coalesce_param<T>... Args>
    //constexpr decltype(auto) coalesce(DT&& default_value, Args&&... to_test_v);
    

    namespace coalesce_impl
    {
        template<typename ValueType, typename PointerType>
        concept raw_pointer_to = requires(ValueType, PointerType pointer)
        {
            {*pointer} -> std::convertible_to<ValueType>;
            {!pointer} noexcept;
        }
        || requires(ValueType value, PointerType pointer)
        {
            {*pointer} -> std::convertible_to<decltype(value())>;
            {!pointer} noexcept;
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


        //template<typename T, typename PT, typename... Args>
        //requires function_returns_ptr<T, PT>
        //constexpr T coalesce(T&& default_value, PT&& to_test_0, Args&&... to_test_v);

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
            return  to_test_0 ? static_cast<RT>(*to_test_0) : coalesce<RT, DT, Args...>(
                std::forward<DT>(default_value),
                std::forward<Args>(to_test_v)...);
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
            return  to_test_0 ? static_cast<RT>(*to_test_0) : coalesce<RT, DT, Args...>(
                std::forward<DT>(default_value),
                std::forward<Args>(to_test_v)...);
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

     
    template<typename RT, typename DT, coalesce_param<RT>... Args>
    requires std::convertible_to< DT, std::remove_reference_t<RT> >
    constexpr decltype(auto) coalesce(DT&& default_value, Args&&... to_test_v)
    {
        return s4::coalesce_impl::coalesce<RT, DT, Args...>(
            std::forward<DT>(default_value),
            std::forward<Args>(to_test_v)...);
    }

    template<typename DT, coalesce_param<DT>... Args>
    requires (!std::invocable<DT>) 
    constexpr decltype(auto) coalesce(DT&& default_value, Args&&... to_test_v)
    {
        return s4::coalesce_impl::coalesce<DT, DT, Args...>(
            std::forward<DT>(default_value), 
            std::forward<Args>(to_test_v)...);
    }

    
    template<typename DT, coalesce_param<DT>... Args>
    requires std::invocable<DT>
    constexpr std::invoke_result_t<DT> coalesce(DT&& default_value, Args&&... to_test_v) 
    {
        return s4::coalesce_impl::coalesce<std::invoke_result_t<DT>, DT, Args...>(
                std::forward<DT>(default_value),
                std::forward<Args>(to_test_v)...) ;
    }
    
} // end namespace s4

#endif
