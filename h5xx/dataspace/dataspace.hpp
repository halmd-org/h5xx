/*
 * Copyright © 2014 Felix Höfling
 * Copyright © 2014 Manuel Dibak
 * Copyright © 2014 Klaus Reuter
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

#ifndef H5XX_DATASPACE_DATASPACE_HPP
#define H5XX_DATASPACE_DATASPACE_HPP

#include <boost/array.hpp>
#include <boost/multi_array.hpp>
#include <boost/type_traits.hpp>
#include <boost/utility/enable_if.hpp>
#include <boost/lexical_cast.hpp>

#include <h5xx/hdf5_compat.hpp>
#include <h5xx/error.hpp>
#include <h5xx/utility.hpp>
#include <h5xx/slice.hpp>

namespace h5xx {

// forward declarations
class attribute;
class dataset;

/**
 * Represents an HDF5 dataspace.
 *
 */
class dataspace
{
public:
    /** default constructor */
    dataspace() : hid_(-1) {}

    /** construct dataspace of rank zero */
    dataspace(H5S_class_t type);

    /** construct simple dataspace of given extents, boost::array version */
    template <std::size_t N>
    dataspace(boost::array<hsize_t, N> const& dims);

    /** construct simple dataspace of given extents, std::vector version */
    dataspace(std::vector<hsize_t> const& dims);

    /** construct simple dataspace of given extents and maximal extents, boost::array version */
    template <std::size_t N>
    dataspace(boost::array<hsize_t, N> const& dims, boost::array<hsize_t, N> const& max_dims);

    /** construct simple dataspace of given extents and maximal extents, std::vector version */
    dataspace(std::vector<hsize_t> const& dims, std::vector<hsize_t> const& max_dims);

// --- generic array versions, cannot be resolved by the compiler currently ---
//    /** construct simple dataspace of given extents */
//    template <class ArrayType>
//    dataspace(ArrayType const& dims);
//
//    /** construct simple dataspace of given extents and maximal extents */
//    template <class ArrayType>
//    dataspace(ArrayType const& dims, ArrayType const& max_dims);

    /**
     * deleted copy constructor
     *
     * Calling the constructor throws an exception. Copying must be elided by
     * return value optimisation. See also "dataspace h5xx::move(dataspace&)".
     */
    dataspace(dataspace const& other);

    /**
     * assignment operator
     *
     * Uses the copy-and-swap idiom. Move semantics is achieved in conjunction
     * with "dataspace h5xx::move(dataspace&)", i.e., the dataspace object on the right
     * hand side is empty after move assignment.
     */
    dataspace const& operator=(dataspace other);

    /** default destructor */
    ~dataspace();

    /** return HDF5 object ID */
    hid_t hid() const
    {
        return hid_;
    }

    /** returns true if associated to a valid HDF5 object */
    bool valid() const
    {
        return hid_ >= 0;
    }

    /** returns rank/dimensionality of simple dataspace */
    int rank() const;

    /** returns extents/dimensions of simple dataspace, optionally the maximal
    *   dimensions are returned in maxdims
    */
    template <std::size_t N>
    boost::array<hsize_t, N> extents(hsize_t *maxdims = NULL) const;

    /** returns extents/dimensions of simple dataspace, optionally the maximal
    *   dimensions are returned in maxdims
    */
    std::vector<hsize_t> extents(hsize_t *maxdims = NULL) const;

    /** returns true if dataspace is of scalar type */
    bool is_scalar() const;

    /** returns true if dataspace is of simple type */
    bool is_simple() const;

    /**
     * TODO : obsolete, use the method "select" in combination with slices
     * simple hyperslab selection, offsets and counts are given as some array
     * type (e.g., boost::array or std::vector)
     */
    template <typename ArrayType>
    void select_hyperslab(ArrayType const& offset, ArrayType const& count);

    /**
     * flags for slice (hyperslab) selection
     */
    enum mode
    {
        SET = H5S_SELECT_SET
      , OR = H5S_SELECT_OR
      , AND = H5S_SELECT_AND
      , XOR = H5S_SELECT_XOR
      , NOTB = H5S_SELECT_NOTB
      , NOTA = H5S_SELECT_NOTA
    };

    /**
     * slice/hyperslab selection interface
     */
    void select(slice const& selection, int mode = SET);

    /**
     * return the number of elements currently selected from the dataspace
     */
    hssize_t get_select_npoints() const;

private:
    /** HDF5 object ID */
    hid_t hid_;

    template <typename h5xxObject>
    friend void swap(h5xxObject& left, h5xxObject& right);

    friend class h5xx::attribute; // method "operator dataspace()"
    friend class h5xx::dataset;   // method "operator dataspace()"
};

dataspace::dataspace(dataspace const& other)
  : hid_(other.hid_)
{
    // copying would be safe if the exception were disabled.
    throw error("h5xx::dataspace can not be copied. Copying must be elided by return value optimisation.");
    H5Iinc_ref(hid_);
}

dataspace const& dataspace::operator=(dataspace other)
{
    swap(*this, other);
    return *this;
}

dataspace::dataspace(H5S_class_t type)
{
    if((hid_ = H5Screate(type)) < 0){
        throw error("creating dataspace");
    }
}

template <std::size_t N>
dataspace::dataspace(boost::array<hsize_t, N> const& dims)
{
    if ((hid_ = H5Screate_simple(N, &*dims.begin(), NULL)) < 0) {
        throw error("creating simple dataspace");
    }
}

template <std::size_t N>
dataspace::dataspace(boost::array<hsize_t, N> const& dims, boost::array<hsize_t, N> const& max_dims)
{
    if ((hid_ = H5Screate_simple(N, &*dims.begin(), &*max_dims.begin())) < 0) {
        throw error("creating simple dataspace");
    }
}

dataspace::dataspace(std::vector<hsize_t> const& dims)
{
    if ((hid_ = H5Screate_simple(dims.size(), &*dims.begin(), NULL)) < 0) {
        throw error("creating simple dataspace");
    }
}

dataspace::dataspace(std::vector<hsize_t> const& dims, std::vector<hsize_t> const& max_dims)
{
    if ((hid_ = H5Screate_simple(dims.size(), &*dims.begin(), &*max_dims.begin())) < 0) {
        throw error("creating simple dataspace");
    }
}


dataspace::~dataspace()
{
    if (hid_ >= 0) {
        if(H5Sclose(hid_) < 0){
            throw error("closing h5xx::dataspace with ID " + boost::lexical_cast<std::string>(hid_));
        }
        hid_ = -1;
    }
}

int dataspace::rank() const
{
    if (!valid()) {
        throw error("invalid dataspace");
    }
    int rank = H5Sget_simple_extent_ndims(hid_);
    if (rank < 0) {
        throw error("dataspace has invalid rank");
    }
    return rank;
}

template <std::size_t N>
boost::array<hsize_t, N> dataspace::extents(hsize_t *maxdims) const
{
    boost::array<hsize_t, N> dims;
    if (rank() != N) {
        throw error("mismatching dataspace rank");
    }
    if (H5Sget_simple_extent_dims(hid_, &*dims.begin(), maxdims) < 0) {
        throw error("determining extents");
    }
    return dims;
}

std::vector<hsize_t> dataspace::extents(hsize_t *maxdims) const
{
    std::vector<hsize_t> dims( rank() );
    if (H5Sget_simple_extent_dims(hid_, &*dims.begin(), maxdims) < 0) {
        throw error("determining extents");
    }
    return dims;
}


bool dataspace::is_scalar() const
{
    if (!valid()) {
        return false;
    }
    return H5Sget_simple_extent_type(hid_) == H5S_SCALAR;
}

bool dataspace::is_simple() const
{
    if (!valid()) {
        return false;
    }
    return H5Sget_simple_extent_type(hid_) == H5S_SIMPLE;
}

// --- routine is obsolete
template <typename ArrayType>
void dataspace::select_hyperslab(ArrayType const& offset, ArrayType const& count)
{
    if (!valid())
        throw error("invalid dataspace");
    if (offset.size() != count.size() || offset.size() != rank())
        throw error("hyperslab specification has mismatching size");
    if (H5Sselect_hyperslab(hid_, H5S_SELECT_SET, &offset[0], NULL, &count[0], NULL) < 0)
        throw error("selecting hyperslab");
}

void dataspace::select(slice const& selection, int mode)
{
    if (!valid())
        throw error("invalid dataspace");
    if (selection.rank() != rank())
        throw error("dataspace and slice have mismatching rank");
    if (H5Sselect_hyperslab(    hid_
                                 , (H5S_seloper_t)mode
                                 , &*selection.get_offset().begin()
                                 , selection.get_stride().size() > 0 ? &*selection.get_stride().begin() : NULL
                                 , &*selection.get_count().begin()
                                 , selection.get_block().size() > 0 ? &*selection.get_block().begin() : NULL
                              ) < 0) {
        throw error("H5Sselect_hyperslab");
    }
}

hssize_t dataspace::get_select_npoints() const
{
    if (!valid())
        throw error("invalid dataspace");
    return H5Sget_select_npoints(hid_);
}

} // namespace h5xx

#endif /* ! H5XX_DATASPACE_DATASPACE_HPP */
