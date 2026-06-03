# Lab 9, directory `D`: to dos for `Rectangle` class

1. Copy over your `C/geometry.txt` into this directory.

2. Create a `rectangle` class.
    * instance variable: `upper_left` (a `Point`)
    * instance variable: `lower_right` (a `Point`)
    * Write a an appropriate constructor based on what you
      learn from the `test_geometry.text` doctest.
    * Note that this doctest builds on from the previous one
      from task/directory `C`.

Note that `Square` (like `Point` and `Circle`) is meant **to be
immutable**. This means that any method which modifies attributes must
result in a new instance of `Square` being created.

3. Add a `__repr__` method
    * Use doctest contents to determine what this should be.

4. Add `area` method
    * no parameters
    * behavior: compute area of rectangle using an appropriate
      formula.

5. Add `perimeter` method
    * no parameters
    * behavior: compute perimeter of rectangle using an appropriate
      formula.

6.  Add `translate` method
    * parameters: `dx` and `dy`
    * behavior: creates and returns a new `Square` instance whose
      anchor points are obtained by modifying those of the
      original `Square` instance and modifying each of those
      points by `dx` and `dy`.
