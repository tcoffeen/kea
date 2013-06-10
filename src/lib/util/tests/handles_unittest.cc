// Copyright (C) 2013  Internet Systems Consortium, Inc. ("ISC")
//
// Permission to use, copy, modify, and/or distribute this software for any
// purpose with or without fee is hereby granted, provided that the above
// copyright notice and this permission notice appear in all copies.
//
// THE SOFTWARE IS PROVIDED "AS IS" AND ISC DISCLAIMS ALL WARRANTIES WITH
// REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY
// AND FITNESS.  IN NO EVENT SHALL ISC BE LIABLE FOR ANY SPECIAL, DIRECT,
// INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM
// LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE
// OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
// PERFORMANCE OF THIS SOFTWARE.

#include <util/hooks/callout_handle.h>
#include <util/hooks/callout_manager.h>
#include <util/hooks/library_handle.h>
#include <util/hooks/server_hooks.h>

#include <boost/lexical_cast.hpp>
#include <boost/scoped_ptr.hpp>
#include <gtest/gtest.h>

#include <algorithm>
#include <string>

/// @file
/// CalloutHandle/LibraryHandle interaction tests
///
/// This file holds unit tests checking the interaction between the
/// CalloutHandle and LibraryHandle[Collection] classes.  In particular,
/// they check that:
///
/// - A CalloutHandle's context is shared between callouts from the same
///   library, but there is a separate context for each library.
///
/// - The various methods manipulating the items in the CalloutHandle's context
///   work correctly.
///
/// - An active callout can only modify the registration of callouts registered
///   by its own library.

using namespace isc::util;
using namespace std;

namespace {

class HandlesTest : public ::testing::Test {
public:
    /// @brief Constructor
    ///
    /// Sets up the various elements used in each test.
    HandlesTest() : hooks_(new ServerHooks()), manager_()
    {
        // Set up four hooks, although through gamma
        alpha_index_ = hooks_->registerHook("alpha");
        beta_index_ = hooks_->registerHook("beta");
        gamma_index_ = hooks_->registerHook("gamma");
        delta_index_ = hooks_->registerHook("delta");

        // Set up for three libraries.
        manager_.reset(new CalloutManager(hooks_, 3));
    }

    /// @brief Return callout manager
    boost::shared_ptr<CalloutManager> getCalloutManager() {
        return (manager_);
    }

    /// Hook indexes - these are frequently accessed, so are accessed directly.
    int alpha_index_;
    int beta_index_;
    int gamma_index_;
    int delta_index_;

private:
    /// Server hooks 
    boost::shared_ptr<ServerHooks> hooks_;

    /// Callout manager.  Declared static so that the callout functions can
    /// access it.
    boost::shared_ptr<CalloutManager> manager_;

};

// The next set of functions define the callouts used by the tests.  They
// manipulate the data in such a way that callouts called - and the order in
// which they were called - can be determined.  The functions also check that
// the "callout context" data areas are separate.
//
// Three libraries are assumed, and each supplies four callouts.  All callouts
// manipulate two context elements the CalloutHandle, the elements being called
// "string" and "int" (which describe the type of data manipulated).
//
// For the string item, each callout shifts data to the left and inserts its own
// data.  The data is a string of the form "nmc", where "n" is the number of
// the library, "m" is the callout number and "y" is the indication of what
// callout handle was passed as an argument ("1" or "2": "0" is used when no
// identification has been set in the callout handle).
//
// For simplicity, and to cut down the number of functions actually written,
// the callout indicator ("1" or "2") ) used in the in the CalloutHandle
// functions is passed via a CalloutArgument.  The argument is named "string":
// use of a name the same as that of one of the context elements serves as a
// check that the argument name space and argument context space are separate.
//
// For integer data, the value starts at zero and an increment is added on each
// call.  This increment is equal to:
//
// 100 * library number + 10 * callout number + callout handle
//
// Although this gives less information than the string value, the reasons for
// using it are:
//
// - It is a separate item in the context, so checks that the context can
//   handle multiple items.
// - It provides an item that can be deleted by the context deletion
//   methods.


// Values set in the CalloutHandle context.  There are three libraries, so
// there are three contexts for the callout, one for each library.

std::string& resultCalloutString(int index) {
    static std::string result_callout_string[3];
    return (result_callout_string[index]);
}

int& resultCalloutInt(int index) {
    static int result_callout_int[3];
    return (result_callout_int[index]);
}

// A simple function to zero the results.

static void zero_results() {
    for (int i = 0; i < 3; ++i) {
        resultCalloutString(i) = "";
        resultCalloutInt(i) = 0;
    }
}


// Library callouts.

// Common code for setting the callout context values.

int
execute(CalloutHandle& callout_handle, int library_num, int callout_num) {

    // Obtain the callout handle number
    int handle_num = 0;
    try  {
        callout_handle.getArgument("handle_num", handle_num);
    } catch (const NoSuchArgument&) {
        // handle_num argument not set: this is the case in the tests where
        // the context_create hook check is tested.
        handle_num = 0;
    }

    // Create the basic data to be appended to the context value.
    int idata = 100 * library_num + 10 * callout_num + handle_num;
    string sdata = boost::lexical_cast<string>(idata);

    // Get the context data. As before, this will not exist for the first
    // callout called. (In real life, the library should create it when the
    // "context_create" hook gets called before any packet processing takes
    // place.)
    int int_value = 0;
    try {
        callout_handle.getContext("int", int_value);
    } catch (const NoSuchCalloutContext&) {
        int_value = 0;
    }

    string string_value = "";
    try {
        callout_handle.getContext("string", string_value);
    } catch (const NoSuchCalloutContext&) {
        string_value = "";
    }

    // Update the values and set them back in the callout context.
    int_value += idata;
    callout_handle.setContext("int", int_value);

    string_value += sdata;
    callout_handle.setContext("string", string_value);

    return (0);
}

// The following functions are the actual callouts - the name is of the
// form "callout_<library number>_<callout number>"

int
callout11(CalloutHandle& callout_handle) {
    return (execute(callout_handle, 1, 1));
}

int
callout12(CalloutHandle& callout_handle) {
    return (execute(callout_handle, 1, 2));
}

int
callout13(CalloutHandle& callout_handle) {
    return (execute(callout_handle, 1, 3));
}

int
callout21(CalloutHandle& callout_handle) {
    return (execute(callout_handle, 2, 1));
}

int
callout22(CalloutHandle& callout_handle) {
    return (execute(callout_handle, 2, 2));
}

int
callout23(CalloutHandle& callout_handle) {
    return (execute(callout_handle, 2, 3));
}

int
callout31(CalloutHandle& callout_handle) {
    return (execute(callout_handle, 3, 1));
}

int
callout32(CalloutHandle& callout_handle) {
    return (execute(callout_handle, 3, 2));
}

int
callout33(CalloutHandle& callout_handle) {
    return (execute(callout_handle, 3, 3));
}

// Common callout code for the fourth hook (which makes the data available for
// checking).  It copies the library and callout context data to the global
// variables.

int printExecute(CalloutHandle& callout_handle, int library_num) {
    callout_handle.getContext("string", resultCalloutString(library_num - 1));
    callout_handle.getContext("int", resultCalloutInt(library_num - 1));

    return (0);
}

// These are the actual callouts.

int
print1(CalloutHandle& callout_handle) {
    return (printExecute(callout_handle, 1));
}

int
print2(CalloutHandle& callout_handle) {
    return (printExecute(callout_handle, 2));
}

int
print3(CalloutHandle& callout_handle) {
    return (printExecute(callout_handle, 3));
}

// This test checks the many-faced nature of the context for the CalloutContext.

TEST_F(HandlesTest, ContextAccessCheck) {
    // Register callouts for the different libraries.
    CalloutHandle handle(getCalloutManager());

    // Library 0.
    getCalloutManager()->setLibraryIndex(0);
    getCalloutManager()->registerCallout("alpha", callout11);
    getCalloutManager()->registerCallout("beta", callout12);
    getCalloutManager()->registerCallout("gamma", callout13);
    getCalloutManager()->registerCallout("delta", print1);

    getCalloutManager()->setLibraryIndex(1);
    getCalloutManager()->registerCallout("alpha", callout21);
    getCalloutManager()->registerCallout("beta", callout22);
    getCalloutManager()->registerCallout("gamma", callout23);
    getCalloutManager()->registerCallout("delta", print2);

    getCalloutManager()->setLibraryIndex(2);
    getCalloutManager()->registerCallout("alpha", callout31);
    getCalloutManager()->registerCallout("beta", callout32);
    getCalloutManager()->registerCallout("gamma", callout33);
    getCalloutManager()->registerCallout("delta", print3);

    // Create the callout handles and distinguish them by setting the
    // "handle_num" argument.
    CalloutHandle callout_handle_1(getCalloutManager());
    callout_handle_1.setArgument("handle_num", static_cast<int>(1)); 

    CalloutHandle callout_handle_2(getCalloutManager());
    callout_handle_2.setArgument("handle_num", static_cast<int>(2)); 

    // Now call the callouts attached to the first three hooks.  Each hook is
    // called twice (once for each callout handle) before the next hook is
    // called.
    getCalloutManager()->callCallouts(alpha_index_, callout_handle_1);
    getCalloutManager()->callCallouts(alpha_index_, callout_handle_2);
    getCalloutManager()->callCallouts(beta_index_, callout_handle_1);
    getCalloutManager()->callCallouts(beta_index_, callout_handle_2);
    getCalloutManager()->callCallouts(gamma_index_, callout_handle_1);
    getCalloutManager()->callCallouts(gamma_index_, callout_handle_2);

    // Get the results for each callout (the callout on hook "delta" copies
    // the context values into a location the test can access).  Explicitly
    // zero the variables before getting the results so we are certain that
    // the values are the results of the callouts.

    zero_results();

    // To explain the expected callout context results.
    //
    // Each callout handle maintains a separate context for each library.  When
    // the first call to callCallouts() is made, "111" gets appended to
    // the context for library 1 maintained by the first callout handle, "211"
    // gets appended to the context maintained for library 2, and "311" to
    // the context maintained for library 3.  In each case, the first digit
    // corresponds to the library number, the second to the callout number and
    // the third to the "handle_num" of the callout handle. For the first call
    // to callCallouts, handle 1 is used, so the last digit is always 1.
    //
    // The next call to callCallouts() calls the same callouts but for the
    // second callout handle.  It also maintains three contexts (one for
    // each library) and they will get "112", "212", "312" appended to
    // them. The explanation for the digits is the same as before, except that
    // in this case, the callout handle is number 2, so the third digit is
    // always 2.  These additions don't affect the contexts maintained by
    // callout handle 1.
    //
    // The process is then repeated for hooks "beta" and "gamma" which, for
    // callout handle 1, append "121", "221" and "321" for hook "beta" and
    // "311", "321" and "331" for hook "gamma".
    //
    // The expected integer values can be found by summing up the values
    // corresponding to the elements of the strings.

    // At this point, we have only called the "print" function for callout
    // handle "1", so the following results are checking the context values
    // maintained in that callout handle.

    getCalloutManager()->callCallouts(delta_index_, callout_handle_1);
    EXPECT_EQ("111121131", resultCalloutString(0));
    EXPECT_EQ("211221231", resultCalloutString(1));
    EXPECT_EQ("311321331", resultCalloutString(2));

    EXPECT_EQ((111 + 121 + 131), resultCalloutInt(0));
    EXPECT_EQ((211 + 221 + 231), resultCalloutInt(1));
    EXPECT_EQ((311 + 321 + 331), resultCalloutInt(2));

    // Repeat the checks for callout 2.

    zero_results();
    getCalloutManager()->callCallouts(delta_index_, callout_handle_2);

    EXPECT_EQ((112 + 122 + 132), resultCalloutInt(0));
    EXPECT_EQ((212 + 222 + 232), resultCalloutInt(1));
    EXPECT_EQ((312 + 322 + 332), resultCalloutInt(2));

    EXPECT_EQ("112122132", resultCalloutString(0));
    EXPECT_EQ("212222232", resultCalloutString(1));
    EXPECT_EQ("312322332", resultCalloutString(2));
}

// Now repeat the test, but add a deletion callout to the list.  The "beta"
// hook of library 2 will have an additional callout to delete the "int"
// element: the same hook for library 3 will delete both elements.  In
// addition, the names of context elements for the libraries at this point
// will be printed.

// List of context item names.

vector<string>&
getItemNames(int index) {
    static vector<string> context_items[3];
    return (context_items[index]);
}

// Context item deletion functions.

int
deleteIntContextItem(CalloutHandle& handle) {
    handle.deleteContext("int");
    return (0);
}

int
deleteAllContextItems(CalloutHandle& handle) {
    handle.deleteAllContext();
    return (0);
}

// Generic print function - copy names in sorted order.

int
printContextNamesExecute(CalloutHandle& handle, int library_num) {
    const int index = library_num - 1;
    getItemNames(index) = handle.getContextNames();
    sort(getItemNames(index).begin(), getItemNames(index).end());
    return (0);
}

int
printContextNames1(CalloutHandle& handle) {
    return (printContextNamesExecute(handle, 1));
}

int
printContextNames2(CalloutHandle& handle) {
    return (printContextNamesExecute(handle, 2));
}

int
printContextNames3(CalloutHandle& handle) {
    return (printContextNamesExecute(handle, 3));
}

// Perform the test including deletion of context items.

TEST_F(HandlesTest, ContextDeletionCheck) {

    getCalloutManager()->setLibraryIndex(0);
    getCalloutManager()->registerCallout("alpha", callout11);
    getCalloutManager()->registerCallout("beta", callout12);
    getCalloutManager()->registerCallout("beta", printContextNames1);
    getCalloutManager()->registerCallout("gamma", callout13);
    getCalloutManager()->registerCallout("delta", print1);

    getCalloutManager()->setLibraryIndex(1);
    getCalloutManager()->registerCallout("alpha", callout21);
    getCalloutManager()->registerCallout("beta", callout22);
    getCalloutManager()->registerCallout("beta", deleteIntContextItem);
    getCalloutManager()->registerCallout("beta", printContextNames2);
    getCalloutManager()->registerCallout("gamma", callout23);
    getCalloutManager()->registerCallout("delta", print2);

    getCalloutManager()->setLibraryIndex(2);
    getCalloutManager()->registerCallout("alpha", callout31);
    getCalloutManager()->registerCallout("beta", callout32);
    getCalloutManager()->registerCallout("beta", deleteAllContextItems);
    getCalloutManager()->registerCallout("beta", printContextNames3);
    getCalloutManager()->registerCallout("gamma", callout33);
    getCalloutManager()->registerCallout("delta", print3);

    // Create the callout handles and distinguish them by setting the "long"
    // argument.
    CalloutHandle callout_handle_1(getCalloutManager());
    callout_handle_1.setArgument("handle_num", static_cast<int>(1));

    CalloutHandle callout_handle_2(getCalloutManager());
    callout_handle_2.setArgument("handle_num", static_cast<int>(2));

    // Now call the callouts attached to the first three hooks.  Each hook is
    // called twice (once for each callout handle) before the next hook is
    // called.
    getCalloutManager()->callCallouts(alpha_index_, callout_handle_1);
    getCalloutManager()->callCallouts(alpha_index_, callout_handle_2);
    getCalloutManager()->callCallouts(beta_index_, callout_handle_1);
    getCalloutManager()->callCallouts(beta_index_, callout_handle_2);
    getCalloutManager()->callCallouts(gamma_index_, callout_handle_1);
    getCalloutManager()->callCallouts(gamma_index_, callout_handle_2);

    // Get the results for each callout.  Explicitly zero the variables before
    // getting the results so we are certain that the values are the results
    // of the callouts.

    zero_results();
    getCalloutManager()->callCallouts(delta_index_, callout_handle_1);

    // The logic by which the expected results are arrived at is described
    // in the ContextAccessCheck test.  The results here are different
    // because context items have been modified along the way.

    EXPECT_EQ((111 + 121 + 131), resultCalloutInt(0));
    EXPECT_EQ((            231), resultCalloutInt(1));
    EXPECT_EQ((            331), resultCalloutInt(2));

    EXPECT_EQ("111121131", resultCalloutString(0));
    EXPECT_EQ("211221231", resultCalloutString(1));
    EXPECT_EQ(      "331", resultCalloutString(2));

    // Repeat the checks for callout handle 2.

    zero_results();
    getCalloutManager()->callCallouts(delta_index_, callout_handle_2);

    EXPECT_EQ((112 + 122 + 132), resultCalloutInt(0));
    EXPECT_EQ((            232), resultCalloutInt(1));
    EXPECT_EQ((            332), resultCalloutInt(2));

    EXPECT_EQ("112122132", resultCalloutString(0));
    EXPECT_EQ("212222232", resultCalloutString(1));
    EXPECT_EQ(      "332", resultCalloutString(2));

    // ... and check what the names of the context items are after the callouts
    // for hook "beta".  We know they are in sorted order.

    EXPECT_EQ(2, getItemNames(0).size());
    EXPECT_EQ(string("int"),    getItemNames(0)[0]);
    EXPECT_EQ(string("string"), getItemNames(0)[1]);

    EXPECT_EQ(1, getItemNames(1).size());
    EXPECT_EQ(string("string"), getItemNames(1)[0]);

    EXPECT_EQ(0, getItemNames(2).size());
}

// Tests that the CalloutHandle's constructor and destructor call the
// context_create and context_destroy callbacks (if registered).  For
// simplicity, we'll use the same callout functions as used above, plus
// the following that returns an error:

int returnError(CalloutHandle&) {
    return (1);
}

TEST_F(HandlesTest, ConstructionDestructionCallouts) {

    // Register context callouts.
    getCalloutManager()->setLibraryIndex(0);
    getCalloutManager()->registerCallout("context_create", callout11);
    getCalloutManager()->registerCallout("context_create", print1);
    getCalloutManager()->registerCallout("context_destroy", callout12);
    getCalloutManager()->registerCallout("context_destroy", print1);

    // Create the CalloutHandle and check that the constructor callout
    // has run.
    zero_results();
    boost::scoped_ptr<CalloutHandle>
        callout_handle(new CalloutHandle(getCalloutManager()));
    EXPECT_EQ("110", resultCalloutString(0));
    EXPECT_EQ(110, resultCalloutInt(0));

    // Check that the destructor callout runs.  Note that the "print1" callout
    // didn't destroy the library context - it only copied it to where it
    // could be examined.  As a result, the destructor callout appends its
    // elements to the constructor's values and the result is printed.
    zero_results();
    callout_handle.reset();

    EXPECT_EQ("110120", resultCalloutString(0));
    EXPECT_EQ((110 + 120), resultCalloutInt(0));

    // Test that the destructor throws an error if the context_destroy
    // callout returns an error. (As the constructor and destructor will
    // have implicitly run the CalloutManager's callCallouts method, we need
    // to set the library index again.)
    getCalloutManager()->setLibraryIndex(0);
    getCalloutManager()->registerCallout("context_destroy", returnError);
    callout_handle.reset(new CalloutHandle(getCalloutManager()));
    EXPECT_THROW(callout_handle.reset(), ContextDestroyFail);

    // We don't know what callout_handle is pointing to - it could be to a
    // half-destroyed object - so use a new CalloutHandle to test construction
    // failure.
    getCalloutManager()->setLibraryIndex(0);
    getCalloutManager()->registerCallout("context_create", returnError);
    boost::scoped_ptr<CalloutHandle> callout_handle2;
    EXPECT_THROW(callout_handle2.reset(new CalloutHandle(getCalloutManager())),
                 ContextCreateFail);
}

// Dynamic callout registration and deregistration.
// The following are the dynamic registration/deregistration callouts.


// Add callout_78_alpha - adds a callout to hook alpha that appends "78x"
// (where "x" is the callout handle) to the current output.

int
callout78(CalloutHandle& callout_handle) {
    return (execute(callout_handle, 7, 8));
}

int
add_callout78_alpha(CalloutHandle& callout_handle) {
    callout_handle.getLibraryHandle().registerCallout("alpha", callout78);
    return (0);
}

int
delete_callout78_alpha(CalloutHandle& callout_handle) {
    static_cast<void>(
        callout_handle.getLibraryHandle().deregisterCallout("alpha",
                                                            callout78));
    return (0);
}

// Check that a callout can register another callout on a different hook.

TEST_F(HandlesTest, DynamicRegistrationAnotherHook) {
    // Register callouts for the different libraries.
    CalloutHandle handle(getCalloutManager());

    // Set up callouts on "alpha".
    getCalloutManager()->setLibraryIndex(0);
    getCalloutManager()->registerCallout("alpha", callout11);
    getCalloutManager()->registerCallout("delta", print1);
    getCalloutManager()->setLibraryIndex(1);
    getCalloutManager()->registerCallout("alpha", callout21);
    getCalloutManager()->registerCallout("delta", print2);
    getCalloutManager()->setLibraryIndex(2);
    getCalloutManager()->registerCallout("alpha", callout31);
    getCalloutManager()->registerCallout("delta", print3);

    // ... and on "beta", set up the function to add a hook to alpha (but only
    // for library 1).
    getCalloutManager()->setLibraryIndex(1);
    getCalloutManager()->registerCallout("beta", add_callout78_alpha);

    // See what we get for calling the callouts on alpha first.
    CalloutHandle callout_handle_1(getCalloutManager());
    callout_handle_1.setArgument("handle_num", static_cast<int>(1)); 
    getCalloutManager()->callCallouts(alpha_index_, callout_handle_1);

    zero_results();
    getCalloutManager()->callCallouts(delta_index_, callout_handle_1);
    EXPECT_EQ("111", resultCalloutString(0));
    EXPECT_EQ("211", resultCalloutString(1));
    EXPECT_EQ("311", resultCalloutString(2));

    // All as expected, now call the callouts on beta.  This should add a
    // callout to the list of callouts for alpha, which we should see when
    // we run the test again.
    getCalloutManager()->callCallouts(beta_index_, callout_handle_1);

    // Use a new callout handle so as to get fresh callout context.
    CalloutHandle callout_handle_2(getCalloutManager());
    callout_handle_2.setArgument("handle_num", static_cast<int>(2)); 
    getCalloutManager()->callCallouts(alpha_index_, callout_handle_2);

    zero_results();
    getCalloutManager()->callCallouts(delta_index_, callout_handle_2);
    EXPECT_EQ("112", resultCalloutString(0));
    EXPECT_EQ("212782", resultCalloutString(1));
    EXPECT_EQ("312", resultCalloutString(2));
}

// Check that a callout can register another callout on the same hook.
// Note that the registration only applies to a subsequent invocation of
// callCallouts, not to the current one. In other words, if
//
// * the callout list for a library is "A then B then C"
// * when callCallouts is executed "B" adds "D" to that list,
//
// ... the current execution of callCallouts only executes A, B and C.  A
// subsequent invocation will execute A, B, C then D.

TEST_F(HandlesTest, DynamicRegistrationSameHook) {
    // Register callouts for the different libraries.
    CalloutHandle handle(getCalloutManager());

    // Set up callouts on "alpha".
    getCalloutManager()->setLibraryIndex(0);
    getCalloutManager()->registerCallout("alpha", callout11);
    getCalloutManager()->registerCallout("alpha", add_callout78_alpha);
    getCalloutManager()->registerCallout("delta", print1);

    // See what we get for calling the callouts on alpha first.
    CalloutHandle callout_handle_1(getCalloutManager());
    callout_handle_1.setArgument("handle_num", static_cast<int>(1)); 
    getCalloutManager()->callCallouts(alpha_index_, callout_handle_1);
    zero_results();
    getCalloutManager()->callCallouts(delta_index_, callout_handle_1);
    EXPECT_EQ("111", resultCalloutString(0));

    // Run it again - we should have added something to this hook.
    CalloutHandle callout_handle_2(getCalloutManager());
    callout_handle_2.setArgument("handle_num", static_cast<int>(2)); 
    getCalloutManager()->callCallouts(alpha_index_, callout_handle_2);
    zero_results();
    getCalloutManager()->callCallouts(delta_index_, callout_handle_2);
    EXPECT_EQ("112782", resultCalloutString(0));

    // And a third time...
    CalloutHandle callout_handle_3(getCalloutManager());
    callout_handle_3.setArgument("handle_num", static_cast<int>(3)); 
    getCalloutManager()->callCallouts(alpha_index_, callout_handle_3);
    zero_results();
    getCalloutManager()->callCallouts(delta_index_, callout_handle_3);
    EXPECT_EQ("113783783", resultCalloutString(0));
}

// Deregistration of a callout from a different hook

TEST_F(HandlesTest, DynamicDeregistrationDifferentHook) {
    // Register callouts for the different libraries.
    CalloutHandle handle(getCalloutManager());

    // Set up callouts on "alpha".
    getCalloutManager()->setLibraryIndex(0);
    getCalloutManager()->registerCallout("alpha", callout11);
    getCalloutManager()->registerCallout("alpha", callout78);
    getCalloutManager()->registerCallout("alpha", callout11);
    getCalloutManager()->registerCallout("delta", print1);

    getCalloutManager()->registerCallout("beta", delete_callout78_alpha);

    // Call the callouts on alpha
    CalloutHandle callout_handle_1(getCalloutManager());
    callout_handle_1.setArgument("handle_num", static_cast<int>(1)); 
    getCalloutManager()->callCallouts(alpha_index_, callout_handle_1);

    zero_results();
    getCalloutManager()->callCallouts(delta_index_, callout_handle_1);
    EXPECT_EQ("111781111", resultCalloutString(0));

    // Run the callouts on hook beta to remove the callout on alpha.
    getCalloutManager()->callCallouts(beta_index_, callout_handle_1);

    // The run of the callouts should have altered the callout list on the
    // first library for hook alpha, so call again to make sure.
    CalloutHandle callout_handle_2(getCalloutManager());
    callout_handle_2.setArgument("handle_num", static_cast<int>(2)); 
    getCalloutManager()->callCallouts(alpha_index_, callout_handle_2);

    zero_results();
    getCalloutManager()->callCallouts(delta_index_, callout_handle_2);
    EXPECT_EQ("112112", resultCalloutString(0));
}

// Deregistration of a callout from the same hook

TEST_F(HandlesTest, DynamicDeregistrationSameHook) {
    // Register callouts for the different libraries.
    CalloutHandle handle(getCalloutManager());

    // Set up callouts on "alpha".
    getCalloutManager()->setLibraryIndex(0);
    getCalloutManager()->registerCallout("alpha", callout11);
    getCalloutManager()->registerCallout("alpha", delete_callout78_alpha);
    getCalloutManager()->registerCallout("alpha", callout78);
    getCalloutManager()->registerCallout("delta", print1);
    getCalloutManager()->setLibraryIndex(1);
    getCalloutManager()->registerCallout("alpha", callout21);
    getCalloutManager()->registerCallout("alpha", callout78);
    getCalloutManager()->registerCallout("delta", print2);

    // Call the callouts on alpha
    CalloutHandle callout_handle_1(getCalloutManager());
    callout_handle_1.setArgument("handle_num", static_cast<int>(1)); 
    getCalloutManager()->callCallouts(alpha_index_, callout_handle_1);

    zero_results();
    getCalloutManager()->callCallouts(delta_index_, callout_handle_1);
    EXPECT_EQ("111781", resultCalloutString(0));
    EXPECT_EQ("211781", resultCalloutString(1));

    // The run of the callouts should have altered the callout list on the
    // first library for hook alpha, so call again to make sure.
    CalloutHandle callout_handle_2(getCalloutManager());
    callout_handle_2.setArgument("handle_num", static_cast<int>(2)); 
    getCalloutManager()->callCallouts(alpha_index_, callout_handle_2);

    zero_results();
    getCalloutManager()->callCallouts(delta_index_, callout_handle_2);
    EXPECT_EQ("112", resultCalloutString(0));
    EXPECT_EQ("212782", resultCalloutString(1));
}


} // Anonymous namespace

