/*  Copyright (c) 2011 Databracket LLC
 *  All rights reserved.
 *
 *  Author: Amr Ali <amr@databracket.com>
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions
 *  are met:
 *  1. Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *  2. Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 *  3. Neither the name of Databracket LLC nor the names of its contributors
 *     may be used to endorse or promote products derived from this software
 *     without specific prior written permission.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 *  IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 *  OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 *  IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 *  INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 *  NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 *  DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 *  THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 *  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 *  THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef __DB_SINGLETON_HH
#define __DB_SINGLETON_HH

#include <memory>
#include <cassert>

namespace databracket
{

    template <class T>
    class singleton {
    public:
        static std::auto_ptr<T>& instance()
        {
            if (!_M_instance.get())
                _M_instance = std::auto_ptr<T>(new T);

            assert(_M_instance.get() != NULL);
            return _M_instance;
        }

    protected:
        singleton();
        ~singleton();

    private:
        singleton(const singleton&);
        singleton& operator =(const singleton&);

    private:
        static std::auto_ptr<T> _M_instance;
    };

    template <class T> std::auto_ptr<T> singleton<T>::_M_instance;

} // namespace databracket

#endif // __DB_SINGLETON_HH
