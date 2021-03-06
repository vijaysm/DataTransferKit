//---------------------------------------------------------------------------//
/*
  Copyright (c) 2012, Stuart R. Slattery
  All rights reserved.

  Redistribution and use in source and binary forms, with or without
  modification, are permitted provided that the following conditions are
  met:

  *: Redistributions of source code must retain the above copyright
  notice, this list of conditions and the following disclaimer.

  *: Redistributions in binary form must reproduce the above copyright
  notice, this list of conditions and the following disclaimer in the
  documentation and/or other materials provided with the distribution.

  *: Neither the name of the University of Wisconsin - Madison nor the
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
 * \brief DTK_FieldMultiVector_impl.hpp
 * \author Stuart R. Slattery
 * \brief MultiVector interface.
 */
//---------------------------------------------------------------------------//

#ifndef DTK_FIELDMULTIVECTOR_IMPL_HPP
#define DTK_FIELDMULTIVECTOR_IMPL_HPP

#include "DTK_DBC.hpp"

#include <Tpetra_Map.hpp>

namespace DataTransferKit
{
//---------------------------------------------------------------------------//
// Constructor.
template<class Scalar>
FieldMultiVector<Scalar>::FieldMultiVector(
    const Teuchos::RCP<Field<Scalar> >& field,
    const Teuchos::RCP<EntitySet>& entity_set )
    : Base( Tpetra::createNonContigMap<int,DofId>(field->getLocalEntityDOFIds(),
						  entity_set->communicator()),
	    field->dimension() )
    , d_field( field )
{ /* ... */ }

//---------------------------------------------------------------------------//
// Pull data from the application and put it in the vector.
template<class Scalar>
void FieldMultiVector<Scalar>::pullDataFromApplication()
{
    Teuchos::ArrayView<const DofId> field_dofs = d_field->getLocalEntityDOFIds();
    Teuchos::ArrayRCP<Teuchos::ArrayRCP<Scalar> > vector_view =
	this->get2dViewNonConst();
    int num_dofs = field_dofs.size();
    int dim = d_field->dimension();
    for ( int n = 0; n < num_dofs; ++n )
    {
	for ( int d = 0; d < dim; ++d )
	{
	    vector_view[d][n] = d_field->readFieldData( field_dofs[n], d );
	}
    }
}

//---------------------------------------------------------------------------//
// Push data from the vector into the application.
template<class Scalar>
void FieldMultiVector<Scalar>::pushDataToApplication()
{
    Teuchos::ArrayView<const DofId> field_dofs = d_field->getLocalEntityDOFIds();
    Teuchos::ArrayRCP<Teuchos::ArrayRCP<const Scalar> > vector_view =
	this->get2dView();
    int num_dofs = field_dofs.size();
    int dim = d_field->dimension();
    for ( int n = 0; n < num_dofs; ++n )
    {
	for ( int d = 0; d < dim; ++d )
	{
	    d_field->writeFieldData( field_dofs[n], d, vector_view[d][n] );
	}
    }

    d_field->finalizeAfterWrite();
}

//---------------------------------------------------------------------------//

} // end namespace DataTransferKit

//---------------------------------------------------------------------------//

#endif // end DTK_FIELDMULTIVECTOR_IMPL_HPP

//---------------------------------------------------------------------------//
// end DTK_FieldMultiVector_impl.hpp
//---------------------------------------------------------------------------//
