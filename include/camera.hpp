#ifndef CAMERA_HPP
#define CAMERA_HPP

#include "gls.hpp"

class Camera
{
  private:
    glm::mat4 proj;
    glm::mat4 view;

  public:
    glm::vec3 position = {0.0f, 0.0f, 0.0f};
    glm::vec3 up = {0.0f, 1.0f, 0.0f};
    glm::vec3 front;

    float yaw = 90.0f;
    float pitch = 0.0f;
    float fov = 45.0f;

    float x, y;
    float w, h;

    float near = 0.5f;
    float far = 1000.0f;

    void update()
    {
        front.x = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
        front.y = sin(glm::radians(pitch));
        front.z = cos(glm::radians(yaw)) * cos(glm::radians(pitch));

        front = glm::normalize(front);
        up = glm::normalize(up);

        proj = glm::perspective(glm::radians(fov), w / h, near, far);
        view = glm::lookAt(position, position + front, up);
    }

    void record()
    {
        glc(glViewport(x, y, w, h));
    }

    const float* proj_gl() const
    {
        return glm::value_ptr(proj);
    }

    const float* view_gl() const
    {
        return glm::value_ptr(view);
    }

    const float* position_gl() const
    {
        return glm::value_ptr(position);
    }

    glm::vec3 right() const
    {
        return glm::normalize(glm::cross(front, up));
    }
};

#endif // CAMERA_HPP