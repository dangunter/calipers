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
Fields in the main nlcali_t struct are accessible (since this is C, after all), but beware that many of them are meaningless until after you have called
`nlcali_calc`.

.. doxygenstruct:: nlcali_t

Summary statistics, i.e. min, mean, etc., are computed for the value,
and two ratios of the value/duration and duration/value. In all cases, the same basic information is kept in he `summary_t`_ structure.

.. doxygenstruct:: summary_t

