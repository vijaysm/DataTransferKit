//---------------------------------------------------------------------------//
/*
  Copyright (c) 2014, Stuart R. Slattery
  All rights reserved.

  Redistribution and use in source and binary forms, with or without
  modification, are permitted provided that the following conditions are
  met:

  *: Redistributions of source code must retain the above copyright
  notice, this list of conditions and the following disclaimer.

  *: Redistributions in binary form must reproduce the above copyright
  notice, this list of conditions and the following disclaimer in the
  documentation and/or other materials provided with the distribution.

  *: Neither the name of the Oak Ridge National Laboratory nor the
  names of its contributors may be used to endorse or promote products
  derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
  A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
  HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
  SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
  LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
  DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
  THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
  OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/
//---------------------------------------------------------------------------//
/*!
 * \brief DTK_EntityIterator.cpp
 * \author Stuart R. Slattery
 * \brief Geometric entity set interface.
 */
//---------------------------------------------------------------------------//

#include "DTK_EntityIterator.hpp"
#include "DTK_DBC.hpp"

namespace DataTransferKit
{
//---------------------------------------------------------------------------//
// Constructor.
EntityIterator::EntityIterator()
{ /* ... */ }

//---------------------------------------------------------------------------//
// Copy constructor.
EntityIterator::EntityIterator( const EntityIterator& rhs )
{
    b_iterator_impl = rhs.clone();
    b_predicate = rhs.b_predicate;
}

//---------------------------------------------------------------------------//
// Assignment operator.
EntityIterator& EntityIterator::operator=( const EntityIterator& rhs )
{
    if ( this == &rhs )
    {
	return *this;
    }
    b_iterator_impl = rhs.clone();
    b_predicate = rhs.b_predicate();
    return *this;
}

//---------------------------------------------------------------------------//
EntityIterator::~EntityIterator()
{ /* ... */ }

//---------------------------------------------------------------------------//
// Pre-increment operator.
EntityIterator& EntityIterator::operator++()
{
    DTK_REQUIRE( Teuchos::nonnull(b_iterator_impl) );
    EntityIterator& it = b_iterator_impl->operator++();
    while ( it != end() )
    {
	if ( !b_predicate(it) )
	{
	    it = b_iterator_impl->operator++();
	}
	else
	{
	    break;
	}
    }
    return it;
}

//---------------------------------------------------------------------------//
// Post-increment operator.
EntityIterator EntityIterator::operator++(int n)
{
    DTK_REQUIRE( Teuchos::nonnull(b_iterator_impl) );
    const EntityIterator tmp = *this;
    EntityIterator it = tmp;
    while ( it != end() )
    {
	if ( !b_predicate(it) )
	{
	    it = b_iterator_impl->operator++();
	}
	else
	{
	    break;
	}
    }
    return tmp;
}

//---------------------------------------------------------------------------//
// Dereference operator.
GeometricEntity& EntityIterator::operator*(void)
{
    DTK_REQUIRE( Teuchos::nonnull(b_iterator_impl) );
    return b_iterator_impl->operator*();
}

//---------------------------------------------------------------------------//
// Dereference operator.
GeometricEntity* EntityIterator::operator->(void)
{
    DTK_REQUIRE( Teuchos::nonnull(b_iterator_impl) );
    return b_iterator_impl->operator->();
}

//---------------------------------------------------------------------------//
// Equal comparison operator.
bool EntityIterator::operator==( const EntityIterator& rhs ) const
{
    DTK_REQUIRE( Teuchos::nonnull(b_iterator_impl) );
    return b_iterator_impl->operator==( rhs );
}

//---------------------------------------------------------------------------//
// Not equal comparison operator.
bool EntityIterator::operator!=( const EntityIterator& rhs ) const
{
    DTK_REQUIRE( Teuchos::nonnull(b_iterator_impl) );
    return b_iterator_impl->operator!=( rhs );
}

//---------------------------------------------------------------------------//

} // end namespace DataTransferKit

//---------------------------------------------------------------------------//
// end DTK_EntityIterator.cpp
//---------------------------------------------------------------------------//
