#include <BitVectors.h>
	
void check_error(bit_vector* c, uint64_t A, uint64_t B, char* op_string,  uint64_t expected,  int* ret_val, uint64_t bit_width)
{
	uint64_t cval = bit_vector_to_uint64(0,c);
	uint64_t expected_val = truncate_uint64(expected,bit_width);

	if(cval != expected_val)
	{
		fprintf(stderr,"Error: A=%llx, B=%llx, %s = %llx, expected = %llx\n", A,B, op_string, cval, expected_val);
		*ret_val = 1;
	}
}

// checks set bit, get_bit
int check_bitsel(uint64_t bit_width)
{

	int ret_val = 0;
	bit_vector tv;
	bit_vector bit_pos;
	bit_vector bit_val;

	init_bit_vector(&tv,bit_width);
	init_bit_vector(&bit_pos, 32);
	init_bit_vector(&bit_val, 1);


	bit_vector_clear(&tv);

	//
	// an elementary march test... checks that
	//    each bit can be set and cleared.
	//    bit-sel correctly reads each bit.
	//
	uint64_t i;
	for(i = 0; i < bit_width; i++)
	{
		bit_vector_set_bit(&tv, i, 1);

		uint64_t j;
		for(j = 0; j < bit_width; j++)
		{
			bit_vector_assign_uint64(0,&bit_pos, j);
			bit_vector_bitsel(&tv, &bit_pos, &bit_val);
			uint8_t bv = bit_vector_get_bit(&tv,j);

			uint8_t bvs = bit_vector_to_uint64(0,&bit_val);

			if(bv != bvs)
			{
				fprintf(stderr,"Error: bit-value mismatch between get_bit and bitsel. (for bit_width=%llu)\n", bit_width);
				ret_val = 1;
			}


			if((i == j) && (bvs != 1))
			{
				fprintf(stderr,"Error: bit-value mismatch in march (for bit_width=%llu, index = %llu\n", bit_width, i);
				ret_val = 1;
			}
			if((i != j) && (bvs != 0))
			{
				fprintf(stderr,"Error: bit-value mismatch in march (for bit_width=%llu, index = %llu\n", bit_width, i);
				ret_val = 1;
			}
		}
		bit_vector_set_bit(&tv, i, 0);
	}

	return(ret_val);
}

//
// concatenate two numbers to produce a larger one.
//
int check_concatenate(uint64_t bit_width)
{
	int ret_val = 0;

	if(bit_width == 1)
		return(0);

	uint64_t s_width = bit_width/2;
	

	__declare_bit_vector(a,bit_width-s_width);
	__declare_bit_vector(b,s_width);
	__declare_bit_vector(c,bit_width-s_width);
	__declare_bit_vector(d,s_width);
	__declare_bit_vector(e,bit_width);


	bit_vector_assign_uint64(0,&a,0xf0);
	uint64_t aval = bit_vector_to_uint64(0,&a);
	bit_vector_assign_uint64(0,&b,0x0f);
	uint64_t bval = bit_vector_to_uint64(0,&b);

	bit_vector_concatenate(&a,&b,&e);

	bit_vector_slice(&e,&c,s_width);
	bit_vector_slice(&e,&d,0);

	if(bit_vector_compare(0,&b,&d) != IS_EQUAL)
	{
		ret_val = 1;
		fprintf(stderr,"Error: in check_concatenate for bit_width=%llu\n", bit_width);
	}
	if(bit_vector_compare(0,&a,&c) != IS_EQUAL)
	{
		ret_val = 1;
		fprintf(stderr,"Error: in check_concatenate for bit_width=%llu\n", bit_width);
	}

	return(ret_val);
}


int check_shifts(uint64_t bit_width)
{

	int ret_val = 0;	

	__declare_bit_vector(a,bit_width);
	bit_vector_set(&a);

	__declare_bit_vector(b,bit_width);
	bit_vector_clear(&b);

	__declare_bit_vector(ab, 2*bit_width);
	bit_vector_concatenate(&a, &b, &ab);

	__declare_bit_vector(e,bit_width);
	bit_vector_assign_uint64(0,&e, bit_width);

	__declare_bit_vector(all_ones,bit_width);
	bit_vector_set(&all_ones);

	__declare_bit_vector(all_zeros,bit_width);
	bit_vector_clear(&all_zeros);

	__declare_bit_vector(rsa_ab,2*bit_width);
	__declare_bit_vector(rsl_ab,2*bit_width);
	__declare_bit_vector(ls_ab,2*bit_width);


	__declare_bit_vector(tmp,bit_width);

	// rsa_ab should have all 1's in the lower half.
	//               all 1's in the upper half.
	bit_vector_shift_right(1, &ab, &e, &rsa_ab); 
	bit_vector_slice(&rsa_ab,&tmp,0);
	if(bit_vector_compare(0,&tmp,&all_ones) != 0)	
	{
		ret_val = 1;
		fprintf(stderr,"Error: shift-right-arithmetic for bit-width %llu, lower half mismatch\n", bit_width);
	}
	bit_vector_slice(&rsa_ab,&tmp,bit_width);
	if(bit_vector_compare(0,&tmp,&all_ones) != 0)	
	{
		ret_val = 1;
		fprintf(stderr,"Error: shift-right-arithmetic for bit-width %llu, upper half mismatch\n", bit_width);
	}



	// rsl_ab should have all 1's in the lower half
	//  and all 0's in the upper half.
	bit_vector_shift_right(0, &ab, &e, &rsl_ab); 
	bit_vector_slice(&rsl_ab,&tmp,0);
	if(bit_vector_compare(0,&tmp,&all_ones) != 0)	
	{
		ret_val = 1;
		fprintf(stderr,"Error: shift-right-logical for bit-width %llu, lower half mismatch\n", bit_width);
	}
	bit_vector_slice(&rsl_ab,&tmp,bit_width);
	if(bit_vector_compare(0,&tmp,&all_zeros) != 0)	
	{
		ret_val = 1;
		fprintf(stderr,"Error: shift-right-logical for bit-width %llu, upper half mismatch\n", bit_width);
	}



	// ls_ab should be full of zeros.
	bit_vector_shift_left (&ab, &e, &ls_ab); 
	bit_vector_slice(&ls_ab,&tmp,0);
	if(bit_vector_compare(0,&tmp,&all_zeros) != 0)	
	{
		ret_val = 1;
		fprintf(stderr,"Error: shift-left for bit-width %llu, lower half mismatch\n", bit_width);
	}
	bit_vector_slice(&rsl_ab,&tmp,bit_width);
	if(bit_vector_compare(0,&tmp,&all_zeros) != 0)	
	{
		ret_val = 1;
		fprintf(stderr,"Error: shift-left for bit-width %llu, upper half mismatch\n", bit_width);
	}

	return(ret_val);
}

int check_compares(uint64_t A, uint64_t B, uint64_t bit_width)
{

	int64_t sA = A;
	int64_t sB = B;
	uint8_t ret_val = 0;
	bit_vector a,b;
	init_bit_vector(&a, bit_width);
	init_bit_vector(&b, bit_width);

	bit_vector_assign_uint64(0,&a,A);
	bit_vector_assign_uint64(0,&b,B);


	bit_vector sa,sb;
	init_bit_vector(&sa, bit_width);
	init_bit_vector(&sb, bit_width);

	bit_vector_assign_uint64(1,&sa,sA);
	bit_vector_assign_uint64(1,&sb,sB);

	uint8_t cmp_result = bit_vector_compare(0,&a,&b);
	uint8_t ucmp = uint64_compare(0,A,B,bit_width);
	if(cmp_result != ucmp)
	{
		ret_val = 1;
		fprintf(stderr,"Error: unsigned compare mismatch for A=%llu, B=%llu, bit_width=%llu\n",A,B,bit_width);
	}

	cmp_result = bit_vector_compare(1,&sa,&sb);
	uint8_t scmp = uint64_compare(1,A,B,bit_width);
	if(cmp_result != scmp)
	{
		ret_val = 1;
		fprintf(stderr,"Error: signed compare mismatch for A=%lld, B=%lld, bit_width=%llu\n",sA,sB,bit_width);
	}

	return(ret_val);
}


int check_if_tests_passed(uint64_t def_size)
{
	int ret_val = 0;
	uint64_t A = rand();
	uint64_t B = rand();

	bit_vector a,b,c,d,e;

	init_bit_vector(&a,def_size);
	init_bit_vector(&b,def_size);
	init_bit_vector(&c,def_size);


	bit_vector_assign_uint64(0,&a,A);
	bit_vector_assign_uint64(0,&b,B);


	bit_vector_assign_uint64(0,&c,0);


	//
	// *,/ will be on truncated A,B values.
	//
	uint64_t Atrunc = truncate_uint64(A, def_size);
	uint64_t Btrunc = truncate_uint64(B, def_size);

	bit_vector_or(&(a),&(b),&(c));
	check_error(&c, A, B, "(A|B)", (A|B), &ret_val, def_size);

	bit_vector_and(&(a),&(b),&(c));
	check_error(&c, A, B, "(A&B)", (A&B), &ret_val, def_size);

	bit_vector_nor(&(a),&(b),&(c));
	check_error(&c, A, B, "~(A|B)", ~(A|B), &ret_val, def_size);

	bit_vector_nand(&(a),&(b),&(c));
	check_error(&c, A, B, "~(A&B)", ~(A&B), &ret_val, def_size);

	bit_vector_xor(&(a),&(b),&(c));
	check_error(&c, A, B, "(A^B)", (A^B), &ret_val, def_size);

	bit_vector_xnor(&(a),&(b),&(c));
	check_error(&c, A, B, "~(A^B)", ~(A^B), &ret_val, def_size);

	bit_vector_plus(&(a),&(b),&(c));
	check_error(&c, A, B, "(A+B)", (A+B), &ret_val, def_size);

	bit_vector_minus(&(a),&(b),&(c));
	check_error(&c, A, B, "(A-B)", (A-B), &ret_val, def_size);

	if(def_size <= 64)
	{
		bit_vector_mul(&(a),&(b),&(c));
		check_error(&c, Atrunc, Btrunc, "(A*B)", (Atrunc*Btrunc), &ret_val, def_size);

		if(Btrunc != 0)
		{
			bit_vector_div(&(a),&(b),&(c));
			check_error(&c, Atrunc, Btrunc, "(A/B)", (Atrunc/Btrunc), &ret_val, def_size);
		}
	}


	ret_val = check_bitsel(def_size) || ret_val;
	ret_val = check_concatenate(def_size) || ret_val;
	ret_val = check_shifts(def_size) || ret_val;
	ret_val = check_compares(truncate_uint64(A,def_size),truncate_uint64(B,def_size),def_size) || ret_val;

	return(ret_val);
}

//
// usage
//    <program>  <lowest-word-length> <highest-word-length>
//
int main(int argc, char* argv[])
{

	srand(119);


	int fail_count = 0;
	uint64_t def_size;

	int L = 1;
	int H = 64;

	if(argc > 1)
		L = atoi(argv[1]);

	if(argc > 2)
		H = atoi(argv[2]);

	for(def_size = L; def_size <= H; def_size++)
	{
		if(check_if_tests_passed(def_size))
		{
			fprintf(stderr,"Error: tests failed for bit-width %llu\n", def_size);
			fail_count++;
		}
	}

	if(fail_count > 0)
		fprintf(stderr,"Error: tests failed for %d bit-widths.\n", fail_count);
	else
		fprintf(stderr,"Info: tests passed for all bit-widths.\n");

	return(fail_count);
}


