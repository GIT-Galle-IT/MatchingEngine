# Building the project
`mkdir build && cd build && cmake .. && make`

# Running ATS after the build
`cd build && ./me`

# Running tests
`cd build && ./run_tests`

# Running benchmark
`cd build && ./benchmark_me`


# Branch Naming Convention

Branches should follow the naming convention:

```
g-it/<type>/<dev-name>/<task-brief>
```

### Type:
- `dev` - Development work
- `test` - Testing work
- ... (Add other relevant types as needed)

### Example:
```
g-it/dev/john-doe/add-user-authentication
```

---

# Commit Naming Convention

Commits should follow the naming convention:

```
<action>[<component name>] - <description>
```

### Action:
- `Add` - For adding new functionality
- `Update` - For modifying existing functionality
- ... (Add other relevant actions as needed)

### Example:
```
Add[Auth] - Implement user login feature
Update[DB] - Optimize query performance
```