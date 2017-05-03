#ifndef QUATERNION
#define QUATERNION

#include "Vector.h"
#include <limits>

namespace tim
{

template<typename T>
class Quaternion {

        static_assert(std::is_floating_point<T>::value, "Quaternion only support floating point types.");

        public:
                Quaternion(const Vector<T,4>& q) : _quat(q.normalized()) {
                }

                template<typename... Args>
                Quaternion(Args&&... args) : _quat(Vector<T,4>(std::forward<Args>(args)...).normalized()) {
                }

                template<typename U>
                Quaternion& operator=(const Vector<T,4>& q) {
                        _quat = q.normalized();
                        return *this;
                }

                template<typename U>
                Quaternion& operator=(const Quaternion<U>& q) {
                        _quat = q._quat;
                        return *this;
                }



                T angle() const {
                        return std::acos(w() * T(2));
                }

                Vector<T,3> axis() const {
                        return Vector<T,3>(_quat.template to<3>() / std::sqrt(T(1) - w() * w()));
                }

                Quaternion inverse() const {
                        return Quaternion(-_quat.template to<3>(), _quat.w());
                }

                explicit operator Vector<T,4>() const {
                        return _quat;
                }

                Vector<T,3> operator()(const Vector<T,3>& v) const {
                        Vector<T,3> u = _quat.template to<3>();
                        return u * T(2) * u.dot(v)
                                  + v * (w() * w() - u.length2())
                                  + u.cross(v) * T(2) * w();
                }

                T x() const {
                         return _quat.x();
                }

                T y() const {
                         return _quat.y();
                }

                T z() const {
                         return _quat.z();
                }

                T w() const {
                         return _quat.w();
                }

                Vector<T,3> to_euler() const {
                        return Vector<T,3>(std::atan2(T(2) * (w() * x() + y() * z()), T(1) - T(2) * (x() * x() + y() * y())),
                                                         std::asin(T(2) * (w() * y() - z() * x())),
                                                         std::atan2(T(2) * (w() * z() + x() * y()), T(1) - T(2) * (y() * y() + z() * z())));
                }

                Vector<T,4> to_axis_angle() const {
                        T s = std::sqrt(T(w) - w() * w());
                        if(s < T(0.001)) {
                                s = T(1);
                        }
                        return Vector<T,4>(_quat.template to<3>() / s, std::acos(w()) * T(2));
                }



                Quaternion operator-() const {
                        return inverse();
                }

                /*template<typename U>
                Quaternion& operator*=(const U& s) const {
                        operator=(*this * s);
                        return *this;
                }

                template<typename U>
                Quaternion& operator/=(const U& s) const {
                        operator=(*this / s);
                        return *this;
                }*/

                Quaternion operator*(const Quaternion& q) const {
                        return Quaternion(w() * q.x() + x() * q.w() + y() * q.z() - z() * q.y(),
                                                          w() * q.y() + y() * q.w() + z() * q.x() - x() * q.z(),
                                                          w() * q.z() + z() * q.w() + x() * q.y() - y() * q.x(),
                                                          w() * q.w() - x() * q.x() - y() * q.y() - z() * q.z());
                }

                Quaternion operator*(T s) const {
                        return Quaternion(_quat * s);
                }

                Quaternion operator/(T s) const {
                        return Quaternion<T>(_quat / s);
                }

                /*static Quaternion look_at(Vector<T,3> f) {
                        f.normalize();
                        Vector<T,3> axis(1, 0, 0);
                        T d = f.dot(axis);
                        if(std::abs(d + T(1.0)) < epsilon<T>()) {
                                return from_axis_angle(Vector<T,3>(0, 0, 1), pi<T>);
                        } else if(std::abs(d - T(1.0)) < epsilon<T>()) {
                                return Quaternion();
                        }
                        return from_axis_angle(f.cross(axis), -std::acos(d));
                }

                static Quaternion from_euler(T yaw, T pitch, T roll) {
                        T cos_yaw = std::cos(pitch * T(0.5));
                        T sin_yaw = std::sin(pitch * T(0.5));
                        T cos_pitch = std::cos(roll * T(0.5));
                        T sin_pitch = std::sin(roll * T(0.5));
                        T cos_roll = std::cos(yaw * T(0.5));
                        T sin_roll = std::sin(yaw * T(0.5));
                        return Quaternion(cos_roll * sin_pitch * cos_yaw + sin_roll * cos_pitch * sin_yaw,
                                                          cos_roll * cos_pitch * sin_yaw - sin_roll * sin_pitch * cos_yaw,
                                                          sin_roll * cos_pitch * cos_yaw - cos_roll * sin_pitch * sin_yaw,
                                                          cos_roll * cos_pitch * cos_yaw + sin_roll * sin_pitch * sin_yaw);
                }

                static Quaternion from_base(const Vector<T,3>& forward, const Vector<T,3>& side, const Vector<T,3>& up) {
                        T w = std::sqrt(1 + forward.x() + side.y() + up.z()) * T(0.5);
                        Vector<T,3> q(side.z() - up.y(), up.x() - forward.z(), forward.y() - side.x());
                        return Quaternion(q / (4 * w), w);
                }

                static Quaternion from_euler(const Vector<T,3>& euler) {
                        return fromEuler(euler[yaw_index], euler[PitchIndex], euler[RollIndex]);
                }*/

                static Quaternion from_axis_angle(const Vector<T,3>& axis, T ang) {
                        T s = std::sin(ang * T(0.5)) / axis.length();
                        return Quaternion(axis.x() * s, axis.y() * s, axis.z() * s, std::cos(ang * T(0.5)));
                }

                /*static Quaternion from_axis_angle(const Vector<T,3>& axis_ang) {
                        return from_axis_angle(axis_ang, axis_ang.length());
                }*/

                static Quaternion from_axis_angle(const Vector<T,4>& v) {
                        return from_axis_angle(v.template to<3>(), v.w());
                }

        private:
                Vector<T,4> _quat;
};

using Quat = Quaternion<float>;

}


#endif // QUATERNION

