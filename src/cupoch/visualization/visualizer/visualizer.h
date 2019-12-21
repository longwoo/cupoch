#pragma once

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <string>

#include <unordered_set>
#include <unordered_map>

#include "cupoch/geometry/geometry.h"
#include "cupoch/visualization/shader/geometry_renderer.h"
#include "cupoch/visualization/utility/color_map.h"
#include "cupoch/visualization/visualizer/render_option.h"
#include "cupoch/visualization/visualizer/view_control.h"

namespace cupoch {

namespace geometry {
class TriangleMesh;
class Image;
}  // namespace geometry

namespace visualization {

class Visualizer {
public:
    struct MouseControl {
    public:
        bool is_mouse_left_button_down = false;
        bool is_mouse_middle_button_down = false;
        bool is_control_key_down = false;
        bool is_shift_key_down = false;
        bool is_alt_key_down = false;
        bool is_super_key_down = false;
        float mouse_position_x = 0.0;
        float mouse_position_y = 0.0;
    };

public:
    Visualizer();
    virtual ~Visualizer();
    Visualizer(Visualizer &&) = delete;
    Visualizer(const Visualizer &) = delete;
    Visualizer &operator=(const Visualizer &) = delete;

public:
    /// Function to create a window and initialize GLFW
    /// This function MUST be called from the main thread.
    bool CreateVisualizerWindow(const std::string &window_name = "Cupoch",
                                const int width = 640,
                                const int height = 480,
                                const int left = 50,
                                const int top = 50,
                                const bool visible = true);

    /// Function to destroy a window
    /// This function MUST be called from the main thread.
    void DestroyVisualizerWindow();

    /// Function to register a callback function for animation
    /// The callback function returns if UpdateGeometry() needs to be run
    void RegisterAnimationCallback(
            std::function<bool(Visualizer *)> callback_func);

    /// Function to activate the window
    /// This function will block the current thread until the window is closed.
    void Run();

    /// Function to to notify the window to be closed
    void Close();

    /// Function to process the event queue and return if the window is closed
    /// Use this function if you want to manage the while loop yourself. This
    /// function will block the thread.
    bool WaitEvents();

    /// Function to process the event queue and return if the window is closed
    /// Use this function if you want to manage the while loop yourself. This
    /// function will NOT block the thread. Thus it is suitable for computation
    /// heavy task behind the scene.
    bool PollEvents();

    /// Function to add geometry to the scene and create corresponding shaders
    /// 1. After calling this function, the Visualizer owns the geometry object.
    /// 2. This function MUST be called after CreateVisualizerWindow().
    /// 3. This function returns FALSE when the geometry is of an unsupported
    /// type.
    /// 4. If an added geometry is changed, the behavior of Visualizer is
    /// undefined. Programmers are responsible for calling UpdateGeometry() to
    /// notify the Visualizer that the geometry has been changed and the
    /// Visualizer should be updated accordingly.
    virtual bool AddGeometry(
            std::shared_ptr<const geometry::Geometry> geometry_ptr,
            bool reset_bounding_box = true);

    /// Function to remove geometry from the scene
    /// 1. After calling this function, the Visualizer releases the pointer of
    /// the geometry object.
    /// 2. This function MUST be called after CreateVisualizerWindow().
    /// 3. This function returns FALSE if the geometry to be removed is not
    /// added by AddGeometry
    virtual bool RemoveGeometry(
            std::shared_ptr<const geometry::Geometry> geometry_ptr,
            bool reset_bounding_box = true);

    /// Function to remove all geometries from the scene.
    /// After calling this function, the Visualizer releases the pointer of
    /// all geometry objects.
    virtual bool ClearGeometries();

    /// Function to update geometry
    /// This function must be called when geometry has been changed. Otherwise
    /// the behavior of Visualizer is undefined.
    /// If called without an argument, updates all geometries, otherwise only
    /// updates the geometry specified.
    virtual bool UpdateGeometry(
            std::shared_ptr<const geometry::Geometry> geometry_ptr = nullptr);
    virtual bool HasGeometry() const;

    /// Function to set the redraw flag as dirty
    virtual void UpdateRender();

    virtual void PrintVisualizerHelp();
    virtual void UpdateWindowTitle();
    virtual void BuildUtilities();

    ViewControl &GetViewControl() { return *view_control_ptr_; }
    RenderOption &GetRenderOption() { return *render_option_ptr_; }

    void ResetViewPoint(bool reset_bounding_box = false);

protected:
    /// Function to do the main rendering
    /// The function first sets view point, then draw geometry (pointclouds and
    /// meshes individually).
    virtual void Render();
    
    // callback functions
    virtual void WindowRefreshCallback(GLFWwindow *window);

protected:
    // window
    GLFWwindow *window_ = NULL;
    std::string window_name_ = "Cupoch";
    std::function<bool(Visualizer *)> animation_callback_func_ = nullptr;
    // Auxiliary internal backup of the callback function.
    // It copies animation_callback_func_ in each PollEvent() or WaitEvent()
    // so that even if user calls RegisterAnimationCallback() within the
    // callback function it is still safe.
    std::function<bool(Visualizer *)> animation_callback_func_in_loop_ =
            nullptr;

    // control
    MouseControl mouse_control_;
    bool is_redraw_required_ = true;
    bool is_initialized_ = false;
    GLuint vao_id_;

    // view control
    std::unique_ptr<ViewControl> view_control_ptr_;

    // rendering properties
    std::unique_ptr<RenderOption> render_option_ptr_;

    // geometry to be rendered
    std::unordered_set<std::shared_ptr<const geometry::Geometry>>
            geometry_ptrs_;

    // geometry renderers
    std::unordered_set<std::shared_ptr<glsl::GeometryRenderer>>
            geometry_renderer_ptrs_;

    // utilities owned by the Visualizer
    std::vector<std::shared_ptr<const geometry::Geometry>> utility_ptrs_;

    // utility renderers
    std::vector<std::shared_ptr<glsl::GeometryRenderer>> utility_renderer_ptrs_;
    // map's key is the renderer for which the RenderOption applies
    // (should be something in utility_renderer_ptrs_)
    std::unordered_map<std::shared_ptr<glsl::GeometryRenderer>, RenderOption>
            utility_renderer_opts_;

    // coordinate frame
    std::shared_ptr<geometry::TriangleMesh> coordinate_frame_mesh_ptr_;
    std::shared_ptr<glsl::CoordinateFrameRenderer>
            coordinate_frame_mesh_renderer_ptr_;
};

}
}