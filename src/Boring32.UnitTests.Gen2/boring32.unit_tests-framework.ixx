export module boring32.unit_tests:framework;
import std;
import std.compat;

namespace unit_tests::results
{
	unsigned successes = 0;
	unsigned failures = 0;
	unsigned total = 0;
	std::chrono::high_resolution_clock::time_point start;
	std::chrono::high_resolution_clock::time_point end;
}

export namespace unit_tests::results
{
	void time_start()
	{
		start = std::chrono::high_resolution_clock::now();
	}

	void time_end()
	{
		end = std::chrono::high_resolution_clock::now();
	}

	void report_success(const std::string& name)
	{
		successes++;
		total++;
	}

	void report_failure(const std::string& name)
	{
		std::println("Test {} failed", name);
		failures++;
		total++;
	}

	void print_results()
	{
		auto duration = end - start;
		std::println("yottaawesome's brand spanking new unit testing framework!");
		std::println("---------------------------------------------------------");
		std::println(
			"All tests completed in {}/{}/{}.",
			std::chrono::duration_cast<std::chrono::milliseconds>(duration),
			std::chrono::duration_cast<std::chrono::microseconds>(duration),
			std::chrono::duration_cast<std::chrono::nanoseconds>(duration)
		);
		std::println("{}/{} succeeded ({} failed).", successes, total, failures);
		std::println("---------------------------------------------------------");
	}
}

export namespace unit_tests::testing
{
	template<typename T>
	concept invocable_or_nullptr = std::invocable<T> or std::is_null_pointer_v<T>;

	// Our basic test type. This is defined more out of utility than
	// necessity, as the testable concept (defined below) doesn't
	// technically require a type to be a template of this type.
	template<std::invocable T, invocable_or_nullptr P = nullptr_t>
	struct test
	{
		std::string name;
		T run;
		P prepare = nullptr;
	};

	// Helper templates for determining whether a given type conforms 
	// to the test concept
	template<typename T>
	struct is_some_test : std::false_type {};

	template<typename T, typename U>
	struct is_some_test<test<T, U>> : std::true_type {};

	// This was originally is_some_test<T>::value, but this unnecessarily constrains
	// it to a template type of test, when the testable concept is wider than that.
	template<typename T>
	concept testable = requires(T t) 
	{ 
		requires std::same_as<decltype(t.name), std::string>;
		t.run(); 
	};

	template<typename T>
	concept preparable_testable = 
		testable<T> 
		and requires(T t) 
		{ 
			{t.prepare()} -> std::same_as<void>; 
		};
	// end

	// Helper templates for determining whether a type is a tuple of tests
	template<typename T>
	struct is_some_tuple_of_tests : std::false_type {};

	template<testable...T>
	struct is_some_tuple_of_tests<std::tuple<T...>> : std::true_type {};

	template<typename T>
	concept testables = is_some_tuple_of_tests<T>::value;

	template<typename T>
	concept test_exporter = requires(T t)
	{
		{t.tests()} -> testables;
	};
	// end

	template<testable...T>
	inline constexpr std::tuple<T...> make_tests(T&&... t)
	{
		return std::tuple{ std::forward<T>(t)... };
	}

	void run(testing::test_exporter auto&&...testers)
	{
		// Concatenate all tests into one tuple
		std::tuple all_tests = std::tuple_cat(testers.tests()...);
		using all_tests_t = decltype(all_tests);

		results::time_start();
		// Forward the all tests tuple on to this lambda and expand for each of its elements
		[]<testables TTuple, size_t...I>(TTuple&& test_tuple, std::index_sequence<I...>)
		{
			// Forward each element sequentially in the test_tuple to this lambda to run the test
			([]<testable TTest>(TTest&& test)
			{
				try
				{
					if constexpr (preparable_testable<TTest>)
						test.prepare();
					test.run();
					results::report_success(test.name);
				}
				catch (...)
				{
					results::report_failure(test.name);
				}
			}(
				std::forward<std::tuple_element_t<I, all_tests_t>>(std::get<I>(test_tuple))
			), ...);
		}(
			std::forward<all_tests_t>(all_tests),
			std::make_index_sequence<std::tuple_size_v<all_tests_t>>{}
		);
		results::time_end();
	}
}

export namespace unit_tests::assert
{
	struct assert_error final : public std::runtime_error
	{
		template<typename...TArgs>
		assert_error(const std::format_string<TArgs...>& fmt, TArgs&&...args)
			: std::runtime_error(std::format(fmt, std::forward<TArgs>(args)...))
		{ }
	};

	inline void is_true(const bool condition, const std::source_location& loc = std::source_location::current())
	{
		if (not condition)
			throw assert_error(
				"Assertion failed at {} in {}:{}.",
				loc.function_name(),
				loc.file_name(),
				loc.line()
			);
	}
}
