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
 * \file DTK_SearchTreeFactory.cpp
 * \author Stuart R. Slattery
 * \brief Search tree factory.
 */
//---------------------------------------------------------------------------//

#include "DTK_SearchTreeFactory.hpp"

namespace DataTransferKit
{
//---------------------------------------------------------------------------//
/*!
 * \brief Creation method.
 *
 * \param dim Spatial dimension of the tree.
 *
 * \param cloud_centers The point cloud coordinates to build the tree with.
 *
 * \param leaf_size The leaft size to build the tree with.
 *
 * \return The constructed tree.
 */
Teuchos::RCP<SearchTree> SearchTreeFactory::create( 
    const unsigned dim,
    const Teuchos::ArrayView<const double>& cloud_centers,
    const unsigned leaf_size )
{
    Teuchos::RCP<SearchTree> tree;

    switch ( dim )
    {
	case 1:
	{
	    tree = Teuchos::rcp( 
		new CloudSearch<1>(cloud_centers, leaf_size) );
	}
	break;

	case 2:
	{
	    tree = Teuchos::rcp( 
		new CloudSearch<2>(cloud_centers, leaf_size) );
	}
	break;

	case 3:
	{
	    tree = Teuchos::rcp( 
		new CloudSearch<3>(cloud_centers, leaf_size) );
	}
	break;
    };

    return tree;
}

//---------------------------------------------------------------------------//

} // end namespace DataTransferKit

//---------------------------------------------------------------------------//
// end DTK_SearchTreeFactory.cpp
//---------------------------------------------------------------------------//

