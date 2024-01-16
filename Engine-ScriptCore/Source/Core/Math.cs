using System;

namespace Engine {

    public struct Vector2 {
        public float x, y;

        public Vector2(float x, float y) {
            this.x = x; 
            this.y = y;
        }

        public override string ToString() {
            return string.Format("({0:F5}, {1:F5})", x, y);
        }

        public float SquaredMagnitude() {
            return x * x + y * y;
        }

        public float Magnitude() {
            return (float)Math.Sqrt(SquaredMagnitude());
        }

        public Vector2 Normalized() {
            return Normalize(this);
        }

        public static Vector2 Zero => new Vector2(0, 0);

        public static Vector2 operator +(Vector2 a, Vector2 b)
            => new Vector2(a.x + b.x, a.y + b.y);

        public static Vector2 operator -(Vector2 a, Vector2 b)
            => new Vector2(a.x - b.x, a.y - b.y);

        public static Vector2 operator *(Vector2 v, float s)
            => new Vector2(v.x * s, v.y * s);

        public static Vector2 Normalize(Vector2 v) {
            float mag = v.Magnitude();
            return new Vector2(v.x / mag, v.y / mag);
        }
    }

    public struct Vector3 {
        public float x, y, z;

        public Vector3(float x, float y, float z) {
            this.x = x;
            this.y = y;
            this.z = z;
        }

        public override string ToString() {
            return string.Format("({0:F5}, {1:F5}, {2:F5})", x, y, z);
        }

        public float SquaredMagnitude() {
            return x * x + y * y + z * z;
        }

        public float Magnitude() {
            return (float)Math.Sqrt(SquaredMagnitude());
        }

        public Vector3 Normalized() {
            return Normalize(this);
        }

        public static Vector3 Zero => new Vector3(0, 0, 0);

        public static Vector3 operator +(Vector3 a, Vector3 b)
            => new Vector3(a.x + b.x, a.y + b.y, a.z + b.z);

        public static Vector3 operator -(Vector3 a, Vector3 b)
            => new Vector3(a.x - b.x, a.y - b.y, a.z - b.z);

        public static Vector3 operator *(Vector3 v, float s)
            => new Vector3(v.x * s, v.y * s, v.z * s);

        public static Vector3 Normalize(Vector3 v) {
            float mag = v.Magnitude();
            return new Vector3(v.x / mag, v.y / mag, v.z / mag);
        }
    }

    public struct Vector4 {
        public float w, x, y, z;

        public Vector4(float x, float y, float z, float w) {
            this.x = x;
            this.y = y;
            this.z = z;
            this.w = w;
        }

        public override string ToString() {
            return string.Format("({0:F5}, {1:F5}, {2:F5}, {3:F5})", x, y, z, w);
        }

        public float SquaredMagnitude() {
            return x * x + y * y + z * z + w * w;
        }

        public float Magnitude() {
            return (float)Math.Sqrt(SquaredMagnitude());
        }

        public Vector4 Normalized() {
            return Normalize(this);
        }

        public static Vector4 Zero => new Vector4(0, 0, 0, 0);

        public static Vector4 operator +(Vector4 a, Vector4 b) 
            => new Vector4(a.x + b.x, a.y + b.y, a.z + b.z, a.w + b.w);

        public static Vector4 operator -(Vector4 a, Vector4 b)
            => new Vector4(a.x - b.x, a.y - b.y, a.z - b.z, a.w - b.w);

        public static Vector4 operator *(Vector4 v, float s) 
            => new Vector4(v.x * s, v.y * s, v.z * s, v.w * s);

        public static Vector4 Normalize(Vector4 v) {
            float mag = v.Magnitude();
            return new Vector4(v.x / mag, v.y / mag, v.z / mag, v.w / mag);
        }
    }

    public struct Quaternion {
        public float w, x, y, z;

        public Quaternion(float x, float y, float z, float w) {
            this.x = x;
            this.y = y;
            this.z = z;
            this.w = w;
        }

        public override string ToString() {
            return string.Format("({0:F5}, {1:F5}, {2:F5}, {3:F5})", x, y, z, w);
        }

        public static Quaternion Identity => new Quaternion(0, 0, 0, 1);

        public static Quaternion Normalize(Quaternion q) {
            // TODO - Calulate normalized quaternion.
            return new Quaternion(0, 0, 0, 0);
        }

        public static Quaternion AngleAxis(float angle, Vector3 axis) {
            // TODO - Calculate quaternion from an angle and an axis.
            return new Quaternion(0, 0, 0, 0);
        }
    }
}