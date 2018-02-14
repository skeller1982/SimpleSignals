# SimpleSignals
simple threadsafe C++ signal/slot implementation

## How it works

The given slot function is wrapped in a std::shared_ptr
The Signal stores a std::weak_ptr to this in a map
The Connect function returns a Connection which owns the slot function
Signal calls the given slot as long as the Connection Object is alive

If you call Disconnect() on the Connection it releases its ownership of the slot, therefore disconnecting from the signal.

If you make copies of the connection the slot function is kept alive until the last connection is destroyed or dicsonnected.

## Usage
- create signal instance, give function parameters as template arguments
- connect any number of functions that have the right signature, you get a connection back
- any .Emit call on the signal calls all connected functions with the  given paremeters

## Example function calls

	Signal<int> mySignal;
	auto connection1 = mySignal.Connect([](int a) {std::cout << a << "a"; });
	auto connection2 = mySignal.Connect([](int b) {std::cout << b << "b"; });
	mySignal.Emit(42);
	connection1.Disconnect();
