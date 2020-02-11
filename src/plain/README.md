Potentially Lame Attempt Not Executed
-------------------------------------

Terminology
- **Node**: an element of a tree structure
  - basic building material of views, created from templates 
  - can have one or more child nodes, and the relationship represents a
    logical/semantic relationship between nodes
  - can have properties attached to them, which can aid in layout and
    presenation
  - immutable and does not contain a reference to its parent
- **Template**: a pure function that are used to create node structures
  - Similar to nodes, templates have zero or more children (child templates)
  - A template can be *applied*, which results in a tree-structure of one or
    more terminal or non-terminal templates
    - Terminal templates cannot be applied further and directly correspond to a
      structure of one or more nodes (never to other templates)
    - Non-terminal templates might yield one or more child templates and is how
    - Unless the templates are recursive it is possible to repeatedly apply a
      non-terminal template and replace it with its result and eventually end
      up with a tree of terminal templates
  - The result of a template is fully determined by its properties, which were
    created by the parent template using an appropriate constructor function.
  - Each template has a name/key that is inherited by all the child templates
    it begets, unless explicitly overridden (cannot give name to oneself)
  - In concrete terms it is made of a data-structure and a few functions
    - construct(alloc, ...): functions that create a context data structure
      that is needed to apply the template, e.g. text for a label, items in a
      list, content of a panel, etc.
    - apply(properties, alloc): a pure function that produces a tree of
      templates, which depends on the properties which were prepared earlier in
      the creation function(s)
    - compare(a, b): compare two sets of properties to see whether *apply*
      would produce the same result as a memoized version.
  - Limited, fixed set of terminal templates; not created outside of library 
- **Layout**: functions that decide size and locations of nodes, based on
  structure and node properties

Outline of PLAIN algorithm
1. **Apply** all *templates* starting from a single root template
  - Recursively evaluate non-terminal templates until only terminal ones remain
  - Repeat until there are no more templates to apply, but memoize what each
    template produced in case partial re-rendering is used
  - Turn terminal templates into nodes
2. **Layout** node into rectangles, based on layout hints in nodes, inherited
   constraints from parent nodes, and global constraints such as window size,
   scaling factor, font sizes, etc.
   - Changing the window size does not necessarily require a change in the
     structure (no template application needed) and often a re-layout is enough
     because the layout algorithm(s) should know if and how a change in
     constraints will affect it
   - Phase one: assuming a certain amount of space is available, how much space
     does the child need?
     - Potential to re-measure earlier nodes after more information has emerged
   - Phase two: Given that the size-requirements of the children are known,
     specify final size and location to child nodes
3. **Expand** nodes into primitive types, now that the size and location of
   each is known based on what is visible in a viewport
   - Solid color rectangles or polygons
   - Filled and stroked spline curves
   - Images (textures)
   - Text boxes (separate pass to render text, perhaps cache rendered result)
   - Pointer group (secondary color buffer that maps each pixel to a pointer-
     sensitive group of nodes which allows input from the screen,
     alternatively, the node tree can be used to lookup what's below the
     pointer)
   - Pointer event emitter: when a pointer event occurs on top of the node or
     any descendent child nodes (unless it further specializes this), emit a
     pointer event for somebody up the chain to handle
   - Event handlers: allow a template to provide logical behavior for its nodes
     that makes logical sense, such as translating click events to button press
     events (filter input events to other events)
4. **Render** primitives by grouping them by type and rendering them in batch
   ideally front to back (except transparent parts)
   - Including pointer-sensitive areas
5. **Process events**: refine events and eventually provide user intent to
   update model
   - Progressively turn primitive events into more and more semantic ones

Design
- Unidirectional data flow (is easier than two-way bindings)
- "Pure" rendering functions; only memory allocations allowed
  - Rendering is a function call: user interface is a function taking a
    model producing a list of renderable things
  - "Semi-space"/linear allocation that can all be free'd in one go
  - The desired parts of the result can be either copied or re-rendered
    with a new allocator.
  - On the slim chance of allocation failure, longjmp to error handler
  -> Code as if allocation never fails, use results directly
  - OOM might be proof of a non-terminating recursive function
- Re-render every frame, or only when model changes, or when particular
  part of model changes, etc.; granularity is not #1 focus right now.
- Rendering output includes other components or primitives
  - Abstract hierarchical/DAG structure, no specific bearing on the visual
    outcome
- Primitives are structural and correspond to renderables
  - Renderable is for output (visual) or to aid input (click areas)
  - The size of the renderable output is not necessarily limited by parents
  - Primitives can be rendered/animated without re-rendering the structure


