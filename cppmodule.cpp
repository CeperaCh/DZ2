#include <vector>
#include <limits>

extern "C" {
#include <Python.h>
}

namespace cppmodule
{
	typedef std::vector<double> row_t;
	typedef std::vector<row_t> matrix_t;

	static matrix_t dot(const matrix_t& M) {
		
		matrix_t A = M;
		size_t size = M.size();
		unsigned int k, i, j;
		for (k = 0; k < size; k++) {
			for (i = 0; i < size; i++) {
				for (j = 0; j < size; j++) {
          double d1 = A[i][j];
          double d2 = A[i][k] + A[k][j];
					if (d1 == 0 || d2 == 0) {
						A[i][j] = 0;
					}
					else if (d1 != std::numeric_limits<double>::infinity() || d2 != std::numeric_limits<double>::infinity()) {
						A[i][j] = 1/(1/d1 + 1/d2);
					}
				}
			}
		}
		return A;
	}

}


static cppmodule::matrix_t pyobject_to_cxx(PyObject * py_matrix)
{
	cppmodule::matrix_t result;
	result.resize(PyObject_Length(py_matrix));
	for (size_t i=0; i<result.size(); ++i) {
		PyObject * py_row = PyList_GetItem(py_matrix, i);
		cppmodule::row_t & row = result[i];
		row.resize(PyObject_Length(py_row));
		for (size_t j=0; j<row.size(); ++j) {
			PyObject * py_elem = PyList_GetItem(py_row, j);
			const double elem = PyFloat_AsDouble(py_elem);
			row[j] = elem;
		}
	}
	return result;
}

static PyObject * cxx_to_pyobject(const cppmodule::matrix_t &matrix)
{
	PyObject * result = PyList_New(matrix.size());
	for (size_t i=0; i<matrix.size(); ++i) {
		const cppmodule::row_t & row = matrix[i];
		PyObject * py_row = PyList_New(row.size());
		PyList_SetItem(result, i, py_row);
		for (size_t j=0; j<row.size(); ++j) {
			const double elem = row[j];
			PyObject * py_elem = PyFloat_FromDouble(elem);
			PyList_SetItem(py_row, j, py_elem);
		}
	}
	return result;
}

static PyObject * matrixops_faster_dot(PyObject * module, PyObject * args)
{
	PyObject * py_a = PyTuple_GetItem(args, 0);

	/* Convert to C++ structure */
	const cppmodule::matrix_t a = pyobject_to_cxx(py_a);

	/* Perform calculations */
	const cppmodule::matrix_t result = cppmodule::dot(a);

	/* Convert back to Python object */
	PyObject * py_result = cxx_to_pyobject(result);
	return py_result;
}

PyMODINIT_FUNC PyInit_cppmodule()
{
	static PyMethodDef ModuleMethods[] = {
		{ "faster_dot", matrixops_faster_dot, METH_VARARGS, "Fater matrix production" },
		{ NULL, NULL, 0, NULL }
	};
	static PyModuleDef ModuleDef = {
		PyModuleDef_HEAD_INIT,
		"cppmodule",
		"Matrix operations",
		-1, ModuleMethods, 
		NULL, NULL, NULL, NULL
	};
	PyObject * module = PyModule_Create(&ModuleDef);
	return module;
}
