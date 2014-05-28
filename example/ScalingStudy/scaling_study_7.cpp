//---------------------------------------------------------------------------//
/*!
 * \file scaling_study_7.cpp
 * \author Stuart R. Slattery
 * \brief Scaling study 7. Mesh-free weak scaling in 3D over cube of points
 * locally random source and target points.
 */
//---------------------------------------------------------------------------//

#include <iostream>
#include <vector>
#include <cmath>
#include <sstream>
#include <algorithm>
#include <cassert>
#include <ctime>
#include <cstdlib>

#include <DTK_MeshFreeInterpolator.hpp>
#include <DTK_MeshFreeInterpolatorFactory.hpp>

#include <Teuchos_GlobalMPISession.hpp>
#include <Teuchos_DefaultComm.hpp>
#include <Teuchos_DefaultMpiComm.hpp>
#include <Teuchos_CommHelpers.hpp>
#include <Teuchos_RCP.hpp>
#include <Teuchos_ArrayRCP.hpp>
#include <Teuchos_OpaqueWrapper.hpp>
#include <Teuchos_Array.hpp>
#include <Teuchos_TypeTraits.hpp>

//---------------------------------------------------------------------------//
// Source function.
//---------------------------------------------------------------------------//
double evaluateSourceFunction( const Teuchos::ArrayView<double>& coords )
{
    return coords[0] + coords[1] + coords[2];
}

//---------------------------------------------------------------------------//
// Overlapping coordinate field.
//---------------------------------------------------------------------------//
Teuchos::ArrayRCP<double> buildOverlappingCoordinates(
    int my_rank, int num_points,
    double i_width, double j_width, double k_width, 
    double i_offset, double j_offset, double k_offset, 
    int seed_add )
{
    std::srand( my_rank*num_points*3 + seed_add );
    int point_dim = 3;
    Teuchos::ArrayRCP<double> coordinate_field(num_points*point_dim);

    for ( int i = 0; i < num_points; ++i )
    {
	coordinate_field[point_dim*i] = 
	    i_width * (double) std::rand() / RAND_MAX + i_offset;
	coordinate_field[point_dim*i + 1] = 
	    j_width * (double) std::rand() / RAND_MAX + j_offset;
	coordinate_field[point_dim*i + 2] = 
	    k_width * (double) std::rand() / RAND_MAX + k_offset;
    }

    return coordinate_field;
}

//---------------------------------------------------------------------------//
// Non-overlapping coordinate field.
//---------------------------------------------------------------------------//
Teuchos::ArrayRCP<double> buildCoordinates(
    int my_rank, int num_points,
    int i_block, int j_block, int k_block, 
    int seed_add )
{
    std::srand( my_rank*num_points*3 + seed_add );
    int point_dim = 3;
    Teuchos::ArrayRCP<double> coordinate_field(num_points*point_dim);

    for ( int i = 0; i < num_points; ++i )
    {
	coordinate_field[point_dim*i] = 
	    (double) std::rand() / RAND_MAX + i_block;
	coordinate_field[point_dim*i + 1] = 
	    (double) std::rand() / RAND_MAX + j_block;
	coordinate_field[point_dim*i + 2] = 
	    (double) std::rand() / RAND_MAX + k_block;
    }

    return coordinate_field;
}

//---------------------------------------------------------------------------//
// Weak scaling study driver.
//---------------------------------------------------------------------------//
int main(int argc, char* argv[])
{
    // Setup communication.
    Teuchos::GlobalMPISession mpiSession(&argc,&argv);
    Teuchos::RCP<const Teuchos::Comm<int> > comm = 
	Teuchos::DefaultComm<int>::getComm();
    int my_rank = comm->getRank();
    int my_size = comm->getSize();

    // Get the block data.
    int num_i_blocks = std::atoi(argv[1]);
    int num_j_blocks = std::atoi(argv[2]);
    int num_k_blocks = std::atoi(argv[3]);
    int k_block = std::floor( my_rank / (num_i_blocks*num_j_blocks) );
    int j_block = std::floor( (my_rank-k_block*num_i_blocks*num_j_blocks) /
			      num_i_blocks );
    int i_block = my_rank - j_block*num_i_blocks - 
		  k_block*num_i_blocks*num_j_blocks;
    assert( my_size == num_i_blocks*num_j_blocks*num_k_blocks );
    assert( i_block < num_i_blocks );
    assert( j_block < num_j_blocks );
    assert( k_block < num_k_blocks );

    int inv_i_block = num_i_blocks - i_block - 1;
    int inv_j_block = num_j_blocks - j_block - 1;
    int inv_k_block = num_k_blocks - k_block - 1;

    // Setup source coordinates.    
    int edge_size = std::atoi(argv[4]);
    int num_points = (edge_size-1)*(edge_size-1)*(edge_size-1)*8;
    assert( 1 < edge_size );
    int overlap = std::atoi(argv[5]);
    assert( -1 < overlap );
    Teuchos::ArrayRCP<double> source_centers;
    if ( overlap ) 
    {
	double i_width = double(num_i_blocks + 1.0) / double(num_i_blocks);
	double j_width = double(num_j_blocks + 1.0) / double(num_j_blocks);
	double k_width = double(num_k_blocks + 1.0) / double(num_k_blocks);
	double i_offset = i_width*i_block - 0.5;
	double j_offset = j_width*j_block - 0.5;
	double k_offset = k_width*k_block - 0.5;
	source_centers =
	    buildOverlappingCoordinates( my_rank, num_points,
					 i_width, j_width, k_width,
					 i_offset, j_offset, k_offset,
					 219381 );
    }
    else
    {
	source_centers = buildCoordinates( my_rank, num_points,
					   i_block, j_block, k_block,
					   98483 );
    }

    // Setup target coordinate field.
    Teuchos::ArrayRCP<double> target_centers = 
	buildCoordinates( my_rank, num_points,
			  inv_i_block, inv_j_block, inv_k_block, 756781 );

    // Evaluate the source function at the source centers.
    Teuchos::Array<double> source_function( num_points );
    for ( int i = 0; i < num_points; ++i )
    {
	source_function[i] = evaluateSourceFunction( source_centers(3*i, 3) );
    }

    // Create data target.
    Teuchos::Array<double> target_function( num_points );

    // Support radius.
    double radius = 1.1 / double(edge_size - 1.0);

    // Interpolation type.
    std::string interpolation_type = argv[6];

    // Basis Type.
    std::string basis_type = argv[7];

    // Interpolation basis order.
    const int basis_order = 4;

    // Build the interpolation object.
    typedef long GlobalOrdinal;
    Teuchos::RCP<DataTransferKit::MeshFreeInterpolator> interpolator =
	DataTransferKit::MeshFreeInterpolatorFactory::create<GlobalOrdinal>(
	    comm, interpolation_type, basis_type, basis_order, 3 );

    // Setup the mesh interpolator.
    std::clock_t setup_start = clock();
    interpolator->setProblem( source_centers(), target_centers(), radius );
    std::clock_t setup_end = clock();

    // Apply the shared domain map ( this does the field evaluation and moves
    // the data ).
    std::clock_t apply_start = clock();
    interpolator->interpolate( source_function(), target_function(), 1 );
    std::clock_t apply_end = clock();

    // Check the data transfer.
    comm->barrier();
    double point_error = 0.0;
    double local_error = 0.0;
    double local_min = 1.0;
    double local_max = 0.0;
    double local_sum = 0.0;
    for ( int i = 0; i < num_points; ++i )
    {
	point_error = std::abs( 
	    (( target_centers[3*i] + target_centers[3*i+1] +
	       target_centers[3*i+2] ) - target_function[i])  / 
	    ( target_centers[3*i] + target_centers[3*i+1] +
	      target_centers[3*i+2] ) );
	local_error += point_error*point_error;
	local_min = std::min( local_min, point_error );
	local_max = std::max( local_max, point_error );
	local_sum += point_error;
    }

    double global_error = 0.0;
    Teuchos::reduceAll<int,double>( *comm,
				    Teuchos::REDUCE_SUM,
				    1,
				    &local_error,
				    &global_error );
    global_error = std::sqrt( global_error );   
    double global_min = 0.0;
    Teuchos::reduceAll<int,double>( *comm,
				    Teuchos::REDUCE_MIN,
				    1,
				    &local_min,
				    &global_min );
    double global_max = 0.0;
    Teuchos::reduceAll<int,double>( *comm,
				    Teuchos::REDUCE_MAX,
				    1,
				    &local_max,
				    &global_max );
    double global_sum = 0.0;
    Teuchos::reduceAll<int,double>( *comm,
				    Teuchos::REDUCE_SUM,
				    1,
				    &local_sum,
				    &global_sum );
    global_sum /= long(num_points) * long(my_size);
    if ( my_rank == 0 )
    {
	std::cout << std::endl << "Error L2 " << global_error << std::endl;
	std::cout << "Error Min " << global_min << std::endl;
	std::cout << "Error Max " << global_max << std::endl;
	std::cout << "Error Ave " << global_sum << std::endl;
    }
    comm->barrier();

    // Timing.
    double local_setup_time = 
    	(double)(setup_end - setup_start) / CLOCKS_PER_SEC;

    double global_min_setup_time;
    Teuchos::reduceAll<int,double>( *comm,
    				    Teuchos::REDUCE_MIN,
    				    1,
    				    &local_setup_time,
    				    &global_min_setup_time );

    double global_max_setup_time;
    Teuchos::reduceAll<int,double>( *comm,
    				    Teuchos::REDUCE_MAX,
    				    1,
    				    &local_setup_time,
    				    &global_max_setup_time );

    double global_average_setup_time;
    Teuchos::reduceAll<int,double>( *comm,
    				    Teuchos::REDUCE_SUM,
    				    1,
    				    &local_setup_time,
    				    &global_average_setup_time );
    global_average_setup_time /= my_size;

    double local_apply_time = 
    	(double)(apply_end - apply_start) / CLOCKS_PER_SEC;

    double global_min_apply_time;
    Teuchos::reduceAll<int,double>( *comm,
    				    Teuchos::REDUCE_MIN,
    				    1,
    				    &local_apply_time,
    				    &global_min_apply_time );

    double global_max_apply_time;
    Teuchos::reduceAll<int,double>( *comm,
    				    Teuchos::REDUCE_MAX,
    				    1,
    				    &local_apply_time,
    				    &global_max_apply_time );

    double global_average_apply_time;
    Teuchos::reduceAll<int,double>( *comm,
    				    Teuchos::REDUCE_SUM,
    				    1,
    				    &local_apply_time,
    				    &global_average_apply_time );
    global_average_apply_time /= my_size;

    comm->barrier();

    if ( my_rank == 0 )
    {
    	std::cout << "==================================================" 
    		  << std::endl;
    	std::cout << "DTK weak scaling study" << std::endl;
    	std::cout << "Number of processors:      " << my_size << std::endl;
    	std::cout << "Local number of points:    " 
		  << num_points << std::endl;
    	std::cout << "Global number of points:   " 
		  << long(num_points)*long(my_size) << std::endl;
    	std::cout << "--------------------------------------------------"
    		  << std::endl;
    	std::cout << "Global min setup time (s):     " 
    		  << global_min_setup_time << std::endl;
    	std::cout << "Global max setup time (s):     " 
    		  << global_max_setup_time << std::endl;
    	std::cout << "Global average setup time (s): " 
    		  << global_average_setup_time << std::endl;
    	std::cout << "--------------------------------------------------"
    		  << std::endl;
    	std::cout << "Global min apply time (s):     " 
    		  << global_min_apply_time << std::endl;
    	std::cout << "Global max apply time (s):     " 
    		  << global_max_apply_time << std::endl;
    	std::cout << "Global average apply time (s): " 
    		  << global_average_apply_time << std::endl;
    	std::cout << "==================================================" 
    		  << std::endl;
    }

    comm->barrier();

    return 0;
}

//---------------------------------------------------------------------------//
// end scaling_study_7.cpp
//---------------------------------------------------------------------------//
