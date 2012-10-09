API
===
.. sectionauthor:: Dan Gunter <dkgunter@lbl.gov>
.. codeauthor:: Dan Gunter <dkgunter@lbl.gov>

This section documents the API for *nl_calipers*.

Constructor
------------

You must first call nlcali_new to create the new *struct nl_cali_t*
object, to which a pointer is returned. This returned pointer is
the first argument to all the other functions, in an OO-style.

.. doxygenfunction:: nlcali_new

All the fields in this structure are accessible (this is C, after all),
see `Structs`_ for details.

Instrumenting
-------------

.. doxygendefine:: nlcali_begin
.. doxygendefine:: nlcali_end

Histogram
---------

To activate the histogram functionality, you must use one of the
methods beginning in `nlcali_hist`.

.. doxygenfunction:: nlcali_hist_manual
.. doxygenfunction:: nlcali_hist_auto

Output
------

.. doxygenfunction:: nlcali_calc
.. doxygenfunction:: nlcali_log
.. doxygenfunction:: nlcali_psdata

Structs
-------
Main data object.
Note that many of the values are meaningless until after you have called
nlcali\_calc(), documented under `Output`_.

.. doxygenstruct:: nlcali_t

Summary statistics data type.

.. doxygenstruct:: nlcali_summ_t

