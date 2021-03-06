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
//----------------------------------*-C++-*----------------------------------//
/*!
 * \file   tstEntityCenteredField.cpp
 * \author Stuart Slattery
 * \date   Wed May 25 12:36:14 2011
 * \brief  Entity-centered DOF vector test.
 */
//---------------------------------------------------------------------------//

#include <iostream>
#include <vector>
#include <cmath>
#include <sstream>

#include "DTK_Point.hpp"
#include "DTK_EntityCenteredField.hpp"
#include "DTK_FieldMultiVector.hpp"
#include "DTK_BasicEntitySet.hpp"

#include "Teuchos_UnitTestHarness.hpp"
#include "Teuchos_RCP.hpp"
#include "Teuchos_Ptr.hpp"
#include "Teuchos_Array.hpp"
#include "Teuchos_ArrayRCP.hpp"
#include "Teuchos_DefaultComm.hpp"

#include <Tpetra_MultiVector.hpp>

//---------------------------------------------------------------------------//
// TESTS
//---------------------------------------------------------------------------//
TEUCHOS_UNIT_TEST( EntityCenteredField, vector_test )
{
    // Initialize parallel communication.
    Teuchos::RCP<const Teuchos::Comm<int> > comm =
	Teuchos::DefaultComm<int>::getComm();

    // Vector parameters.
    int num_vec = 3;
    int vec_length = 10;

    // Create an entity set.
    Teuchos::RCP<DataTransferKit::BasicEntitySet> entity_set =
	Teuchos::rcp( new DataTransferKit::BasicEntitySet(comm,1) );
    
    // Create data. 
    Teuchos::Array<double> coords(1);
    Teuchos::Array<DataTransferKit::Entity> points( vec_length );
    Teuchos::ArrayRCP<double> in_data( num_vec *vec_length );
    Teuchos::ArrayRCP<double> out_data( num_vec *vec_length );
    for ( int i = 0; i < vec_length; ++i )
    {
	coords[0] = i;
	points[i] = DataTransferKit::Point( i+1, comm->getRank(), coords );
	for ( int d = 0; d < num_vec; ++d )
	{
	    in_data[d*vec_length + i] = 2.0*i + 1.0;
	    out_data[d*vec_length + i] = 0.0;
	}
    }

    // Create an input field.
    Teuchos::RCP<DataTransferKit::Field<double> > in_field =
	Teuchos::rcp( new DataTransferKit::EntityCenteredField<double>(
			  points(), num_vec, in_data,
			  DataTransferKit::EntityCenteredField<double>::BLOCKED) );

    // Create an input vector.
    Teuchos::RCP<Tpetra::MultiVector<double,int,std::size_t> > in_vec =
	Teuchos::rcp( new DataTransferKit::FieldMultiVector<double>(
			  in_field, entity_set) );

    // Create an output field.
    Teuchos::RCP<DataTransferKit::Field<double> > out_field =
	Teuchos::rcp( new DataTransferKit::EntityCenteredField<double>(
			  points(), num_vec, out_data,
			  DataTransferKit::EntityCenteredField<double>::BLOCKED) );

    // Create an output vector.
    Teuchos::RCP<Tpetra::MultiVector<double,int,std::size_t> > out_vec =
	Teuchos::rcp( new DataTransferKit::FieldMultiVector<double>(
			  out_field, entity_set) );

    // Pull in the results.
    Teuchos::rcp_dynamic_cast<DataTransferKit::FieldMultiVector<double> >(
	in_vec)->pullDataFromApplication();
    
    // Add the vectors together.
    out_vec->update( 1.0, *in_vec, 0.0 );

    // Push back the results.
    Teuchos::rcp_dynamic_cast<DataTransferKit::FieldMultiVector<double> >(
	out_vec)->pushDataToApplication();

    // Check the results.
    for ( int i = 0; i < vec_length; ++i )
    {
	for ( int d = 0; d < num_vec; ++d )
	{
	    TEST_EQUALITY( in_data[d*vec_length + i],
			   out_data[d*vec_length + i] );
	}
    }
}

//---------------------------------------------------------------------------//
// end of tstEntityCenteredField.cpp
//---------------------------------------------------------------------------//
