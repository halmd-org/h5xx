/*
 * Copyright © 2010-2013  Felix Höfling
 *
 * This file is part of h5xx.
 *
 * h5xx is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#define BOOST_TEST_MODULE h5xx_attribute
#include <boost/test/unit_test.hpp>

#include <h5xx/h5xx.hpp>

#include <boost/make_shared.hpp>
#include <cmath>
#include <unistd.h>

#include <test/ctest_full_output.hpp>
#include <test/catch_boost_no_throw.hpp>

BOOST_GLOBAL_FIXTURE( ctest_full_output )

BOOST_AUTO_TEST_CASE( h5xx_attribute )
{
    char const filename[] = "test_h5xx_attribute.hdf5";

    // store H5File object in shared_ptr to destroy it before re-opening the file

    boost::shared_ptr<H5::H5File> file;
    H5::Group group;
    BOOST_CHECK_NO_THROW(file = boost::make_shared<H5::H5File>(filename, H5F_ACC_TRUNC));
    BOOST_CHECK_NO_THROW(group = h5xx::open_group(*file, "/"));

    bool bool_value = true;
    BOOST_CHECK_NO_THROW(h5xx::write_attribute(group, "bool, scalar", bool_value));
    BOOST_CHECK(h5xx::exists_attribute(group, "bool, scalar"));
    BOOST_CHECK_NO_THROW(h5xx::delete_attribute(group, "bool, scalar"));
    BOOST_CHECK(h5xx::exists_attribute(group, "bool, scalar") == false);
    BOOST_CHECK_NO_THROW(h5xx::write_attribute(group, "bool, scalar", bool_value));

    uint64_t uint_value = 9223372036854775783LLU;  // largest prime below 2^63
    BOOST_CHECK_NO_THROW(h5xx::write_attribute(group, "integral, scalar", uint64_t(1)));   // store wrong value of correct type first
    BOOST_CHECK_NO_THROW(h5xx::write_attribute(group, "integral, scalar", uint_value));  // overwrite value

    // long double is supported by the HDF5 library,
    // but neither by h5dump nor pytables ...
    long double ldouble_value = std::sqrt(2.L);
    BOOST_CHECK_NO_THROW(h5xx::write_attribute(group, "long double, scalar", 2));   // store something of wrong type first
    BOOST_CHECK_NO_THROW(h5xx::write_attribute(group, "long double, scalar", ldouble_value));
    BOOST_CHECK_NO_THROW(h5xx::write_attribute(group, "double, scalar", static_cast<double>(ldouble_value)));

    boost::array<char const*, 3> cstring_array = {{
        "HALMD", "HAL's MD package",
        "Highly accelerated large-scale molecular dynamics simulation package"
    }};
    typedef boost::array<std::string, 3> string_array_type;
    BOOST_CHECK_NO_THROW(h5xx::write_attribute(group, "char [], scalar", cstring_array[1]));
    BOOST_CHECK_NO_THROW(h5xx::write_attribute(group, "string, scalar", std::string(cstring_array[1])));
    BOOST_CHECK_NO_THROW(h5xx::write_attribute(group, "char [], array", cstring_array));

    typedef boost::array<bool, 2> bool_array_type;
    bool_array_type bool_array = {{ true, false }};
    BOOST_CHECK_NO_THROW(h5xx::write_attribute(group, "bool, array", bool_array));

    typedef boost::array<double, 5> double_array_type;
    double_array_type value_array;
    double double_values[] = {
       1., std::sqrt(2.), 2., std::sqrt(3.), 3.
    };
    std::copy(double_values, double_values + value_array.size(), value_array.begin());
    BOOST_CHECK_NO_THROW(h5xx::write_attribute(group, "double, array", value_array));

    typedef std::vector<double> double_vector_type;
    double_vector_type value_vector(value_array.size());
    std::copy(value_array.begin(), value_array.end(), value_vector.begin());
    BOOST_CHECK_NO_THROW(h5xx::write_attribute(group, "double, std::vector", value_vector));
    value_vector.resize(4);
    BOOST_CHECK_NO_THROW(h5xx::write_attribute(group, "double, std::vector", value_vector));     // overwrite with different size

    typedef boost::multi_array<int, 3> multi_array3;
    int data3[] = {
        99,98,97,96,
        95,94,93,92,
        91,90,89,88,

        87,86,85,84,
        83,82,81,80,
        79,78,77,76
    };
    multi_array3 multi_array_value(boost::extents[2][3][4]);
    multi_array_value.assign(data3, data3 + 2 * 3 * 4);
    BOOST_CHECK_NO_THROW(h5xx::write_attribute(group, "int, multi_array", multi_array_value));

    // re-open file
    BOOST_CHECK_NO_THROW(group.close());
    BOOST_CHECK_NO_THROW(file = boost::make_shared<H5::H5File>(filename, H5F_ACC_RDONLY));
    BOOST_CHECK_NO_THROW(group = h5xx::open_group(*file, "/"));

    // check h5xx::has_type<>
    BOOST_CHECK(h5xx::has_type<bool>(group.openAttribute("bool, scalar")));
    BOOST_CHECK(h5xx::has_type<uint64_t>(group.openAttribute("integral, scalar")));
    BOOST_CHECK(h5xx::has_type<long double>(group.openAttribute("long double, scalar")));
    BOOST_CHECK(h5xx::has_type<double>(group.openAttribute("double, scalar")));
    BOOST_CHECK(h5xx::has_type<char const*>(group.openAttribute("char [], scalar")));
    BOOST_CHECK(h5xx::has_type<std::string>(group.openAttribute("string, scalar")));
    BOOST_CHECK(h5xx::has_type<string_array_type>(group.openAttribute("char [], array")));
    BOOST_CHECK(h5xx::has_type<bool_array_type>(group.openAttribute("bool, array")));
    BOOST_CHECK(h5xx::has_type<double_array_type>(group.openAttribute("double, array")));
    BOOST_CHECK(h5xx::has_type<double_vector_type>(group.openAttribute("double, std::vector")));
    BOOST_CHECK(h5xx::has_type<multi_array3>(group.openAttribute("int, multi_array")));

    // check h5xx::has_scalar_space()
    BOOST_CHECK(h5xx::has_scalar_space(group.openAttribute("bool, scalar")));
    BOOST_CHECK(h5xx::has_scalar_space(group.openAttribute("integral, scalar")));
    BOOST_CHECK(h5xx::has_scalar_space(group.openAttribute("long double, scalar")));
    BOOST_CHECK(h5xx::has_scalar_space(group.openAttribute("double, scalar")));
    BOOST_CHECK(h5xx::has_scalar_space(group.openAttribute("char [], scalar")));
    BOOST_CHECK(h5xx::has_scalar_space(group.openAttribute("string, scalar")));

    // check h5xx::has_extent<>
    BOOST_CHECK(h5xx::has_extent<string_array_type>(group.openAttribute("char [], array")));
    BOOST_CHECK(h5xx::has_extent<bool_array_type>(group.openAttribute("bool, array")));
    BOOST_CHECK(h5xx::has_extent<double_array_type>(group.openAttribute("double, array")));
    BOOST_CHECK(h5xx::has_extent<multi_array3>(group.openAttribute("int, multi_array"),
                multi_array_value.shape()));

    // read attributes
    BOOST_CHECK(h5xx::read_attribute<bool>(group, "bool, scalar") == bool_value);
    BOOST_CHECK(h5xx::read_attribute<uint64_t>(group, "integral, scalar") == uint_value);
    BOOST_CHECK(h5xx::read_attribute<long double>(group, "long double, scalar") == ldouble_value);
    BOOST_CHECK(h5xx::read_attribute<double>(group, "double, scalar")
                == static_cast<double>(ldouble_value));
    BOOST_CHECK(h5xx::read_attribute<std::string>(group, "char [], scalar") == cstring_array[1]);
    BOOST_CHECK(h5xx::read_attribute<std::string>(group, "string, scalar") == cstring_array[1]);
    // read support for fixed-size string array is missing
//     BOOST_CHECK(h5xx::read_attribute<const char*>(group, "char [], array") == cstring_array);
    BOOST_CHECK(h5xx::read_attribute<bool_array_type>(group, "bool, array") == bool_array);
    BOOST_CHECK(h5xx::read_attribute<double_array_type>(group, "double, array") == value_array);
    value_vector = h5xx::read_attribute<double_vector_type>(group, "double, array");
    BOOST_CHECK(equal(value_vector.begin(), value_vector.end(), value_array.begin()));
    BOOST_CHECK(h5xx::read_attribute<multi_array3>(group, "int, multi_array") == multi_array_value);
    // read multi_array as linear vector
    std::vector<int> int_vector = h5xx::read_attribute<std::vector<int> >(group, "int, multi_array");
    BOOST_CHECK(int_vector.size() == multi_array_value.num_elements());
    BOOST_CHECK(equal(int_vector.begin(), int_vector.end(), multi_array_value.origin()));

    // check exists_attribute
    BOOST_CHECK(h5xx::exists_attribute(group, "bool, scalar") == true);
    BOOST_CHECK(h5xx::exists_attribute(group, "foo") == false);
    H5E_BEGIN_TRY {
        BOOST_CHECK_THROW(h5xx::exists_attribute(group, "") == false, h5xx::error);
        BOOST_CHECK_THROW(h5xx::exists_attribute(H5::Group(-1), "foo"), h5xx::error);
    } H5E_END_TRY

    // check delete_attribute
    H5E_BEGIN_TRY{
         BOOST_CHECK_THROW(h5xx::delete_attribute(group, "bool, scalar"), h5xx::error); // delete deleted attribute in H5_ACC_RDONLY access mode
     } H5E_END_TRY

    // remove file
#ifdef NDEBUG
    file.reset();
    unlink(filename);
#endif
}
