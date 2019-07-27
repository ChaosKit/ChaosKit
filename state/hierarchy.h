#ifndef CHAOSKIT_STATE_HIERARCHY_H
#define CHAOSKIT_STATE_HIERARCHY_H

#include <type_traits>
#include <vector>

/**
 * Macro that specifies the Relation and ParentOf templates to correctly resolve
 * all the types for the specified relationship.
 *
 * Usage:
 * CHAOSKIT_DEFINE_RELATION(Parent, Child, &Parent::child);
 */
#define CHAOSKIT_DEFINE_RELATION(ParentType, ChildType, ChildField) \
  namespace chaoskit::state {                              \
  template <>                                              \
  struct Relation<ParentType, ChildType> {                 \
    using Parent = ParentType;                             \
    using Child = ChildType;                               \
    using FieldPtr = decltype(ChildField);                 \
    constexpr static FieldPtr field = ChildField;          \
  };                                                       \
  template <>                                              \
  struct ParentOf<ChildType> {                             \
    using Type = ParentType;                               \
  };                                                       \
  }

/**
 * Additional macro that specifies the ChildrenOf template with the list of
 * children for the given ParentType.
 *
 * Usage:
 * CHAOSKIT_DEFINE_CHILDREN(Parent, Child, AnotherChild);
 */
#define CHAOSKIT_DEFINE_CHILDREN(ParentType, ...)       \
  namespace chaoskit::state {                           \
  template <>                                           \
  struct ChildrenOf<ParentType> {                       \
    using List = ChildList<ParentType, __VA_ARGS__>;    \
    inline constexpr static size_t count = List::count; \
  };                                                    \
  }

namespace chaoskit::state {

namespace detail {
template <typename>
struct IsVector : std::false_type {};
template <typename T, typename A>
struct IsVector<std::vector<T, A>> : std::true_type {};
}  // namespace detail

/** Structure that tells the type of the parent of a given type. */
template <typename C>
struct ParentOf {
  using Type = void;
};

/**
 * Structure that tells the type and location of a field containing the parent's
 * children of type C.
 */
template <typename P, typename C>
struct Relation {
  using Parent = void;
  using Child = void;
  using FieldPtr = void*;
  constexpr static FieldPtr field = nullptr;
};

/**
 * Checks if P and C are in a valid relation, i.e. if P is the parent of C.
 */
template <typename P, typename C>
constexpr bool isValidRelation() {
  using CD = Relation<P, C>;
  return !std::is_void_v<typename CD::Parent> &&
         !std::is_void_v<typename CD::Child>;
}

/** A list of types Cs that are defined as children of P. */
template <typename P, typename... Cs>
struct ChildList;

template <typename P, typename C, typename... Cs>
struct ChildList<P, C, Cs...> {
  inline constexpr static size_t count = sizeof...(Cs) + 1;

  /**
   * Calls the supplied callable for each type that is a child of P.
   *
   * The first argument passed to the callable is a null pointer of type C*.
   * This is used only for the purpose of picking proper type overloads.
   */
  template <typename Fn>
  constexpr static void forEachChild(Fn&& fn) {
    if constexpr (isValidRelation<P, C>()) {
      fn(static_cast<C*>(nullptr));
    }
    ChildList<P, Cs...>::forEachChild(std::forward<Fn>(fn));
  }
};

template <typename P>
struct ChildList<P> {
  constexpr static size_t count = 0;

  template <typename Fn>
  static void forEachChild(Fn&& fn) {}
};

/** Structure that tells the types and count of all children types of P. */
template <typename P>
struct ChildrenOf {
  using List = ChildList<P>;
  constexpr static size_t count = 0;
};

/** Checks if P is a parent of anything. */
template <typename P>
constexpr bool isParent() {
  return ChildrenOf<P>::count > 0;
}

/** Checks if P is a child of anything. */
template <typename C>
constexpr bool isChild() {
  return !std::is_void_v<typename ParentOf<C>::Type>;
}

/** Checks if P is a parent of C. */
template <typename C, typename P>
constexpr bool isParentOf() {
  return std::is_same_v<typename Relation<P, C>::Parent, P>;
}

/** Checks if C is a child of P. */
template <typename P, typename C>
constexpr bool isChildOf() {
  return std::is_same_v<typename Relation<P, C>::Child, C>;
}

/**
 * Checks if P has a has-many relationship with C, i.e. if P's children are
 * stored in a vector.
 */
template <typename P, typename C>
constexpr bool isHasManyRelationship() {
  static_assert(isValidRelation<P, C>());
  return detail::IsVector<
      std::remove_reference_t<decltype(P().*(Relation<P, C>::field))>>::value;
}

/**
 * Checks if P has a has-one relationship with C, i.e. if P's child is stored
 * in a pointer.
 */
template <typename P, typename C>
constexpr bool isHasOneRelationship() {
  static_assert(isValidRelation<P, C>());
  return std::is_pointer_v<
      std::remove_reference_t<decltype(P().*(Relation<P, C>::field))>>;
}

/**
 * Returns a reference to the field containing the children of the parent.
 *
 * It's either a reference to a vector or a reference to a pointer, depending on
 * whether P and C are in a has-many or has-one relationship.
 */
template <typename C, typename P>
decltype(auto) getChildren(P& parent) {
  static_assert(isValidRelation<P, C>());
  return parent.*(Relation<P, C>::field);
}

/**
 * Associates a child with a parent.
 *
 * In case of a has-one relationship, this sets the child pointer to the given
 * child. In case of a has-many relationship, this adds the child to the vector
 * of children.
 */
template <typename P, typename C>
void associateChild(P& parent, C* child) {
  static_assert(isParentOf<C, P>());
  if constexpr (isHasManyRelationship<P, C>()) {
    getChildren<C>(parent).push_back(child);
  } else {
    getChildren<C>(parent) = child;
  }
}

/**
 * Dissociates a child with a parent.
 *
 * In case of a has-one relationship, this sets the child pointer to null. In
 * case of a has-many relationship, this removes the child pointer from the
 * vector of children.
 */
template <typename P, typename C>
void dissociateChild(P& parent, C* child) {
  static_assert(isParentOf<C, P>());
  if constexpr (isHasManyRelationship<P, C>()) {
    auto& children = getChildren<C>(parent);
    children.erase(std::remove(std::begin(children), std::end(children), child),
                   std::end(children));
  } else {
    auto& children = getChildren<C>(parent);
    if (children == child) {
      children = nullptr;
    }
  }
}

}  // namespace chaoskit::state

#endif  // CHAOSKIT_STATE_HIERARCHY_H
