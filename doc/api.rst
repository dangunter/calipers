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
but beware that many of them are meaningless until after you have called
`nlcali_calc()`, documented below.

.. doxygenstruct:: nlcali_t

Histogram
---------

To activate the histogram functionality, you must use one of the
methods beginning in `nlcali_hist`.

.. doxygenfunction:: nlcali_hist_manual
.. doxygenfunction:: nlcali_hist_auto

Instrumenting
-------------

.. doxygendefine:: nlcali_begin
.. doxygendefine:: nlcali_end

Output
------

.. doxygenfunction:: nlcali_calc
.. doxygenfunction:: nlcali_log
.. doxygenfunction:: nlcali_psdata