#pragma once

#include <iostream>
#include <memory>
#include <functional>

#include "framework.h"

namespace sorokin
{

    class WindowsFtpClient
    {
        template<class T>
        using custom_delete_unique_ptr = std::unique_ptr<T, std::function<void( T* )>>;

    public:
        WindowsFtpClient() : 
            pSession_(nullptr, []( CInternetSession* ptr )
                {
                    if ( !ptr ) return;
                    ptr->Close();
                    delete ptr;
                } ),
            pConnection_( nullptr, []( CFtpConnection* ptr )
                {
                    if ( !ptr ) return;
                    ptr->Close();
                } )
        {}
        ~WindowsFtpClient() noexcept
        {
            if ( IsOpen() ) try
            {
                Close();
            }
            catch ( ... )
            {
                std::cout << "Caught unknown exception on disconnect.\n";
            }
        }

        WindowsFtpClient( const WindowsFtpClient& ) = delete;
        WindowsFtpClient( WindowsFtpClient&& ) = delete;
        WindowsFtpClient& operator=( const WindowsFtpClient& ) = delete;
        WindowsFtpClient& operator=( WindowsFtpClient&& ) = delete;

        bool Open(CString login, CString pwd, 
            CString ip, INTERNET_PORT port = INTERNET_DEFAULT_FTP_PORT,
            BOOL passive_flag = FALSE );
        [[nodiscard]] bool IsOpen() const noexcept
        {
            return pSession_.operator bool();
        }

        bool ExecuteSimpleCommand( const wchar_t* command );
        bool GetExecuteResult();

        void Close()
        {
            pSession_.reset( nullptr );
            pConnection_.reset( nullptr );
        }

    private:
        custom_delete_unique_ptr<CInternetSession> pSession_;
        custom_delete_unique_ptr<CFtpConnection> pConnection_;
        static constexpr std::size_t kErrorBufferSize = 1024;

    private:
        static void ProcessError( const CInternetException* const error );
    };

} // namespace sorokin