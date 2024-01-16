using System;
using System.Runtime.CompilerServices;

namespace Engine {

    public static class InternalCalls {

        #region TransformComponent
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void TransformComponent_GetPosition(ulong eID, out Vector3 position);
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void TransformComponent_SetPosition(ulong eID, ref Vector3 position);
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void TransformComponent_GetRotation(ulong eID, out Quaternion rotation);
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void TransformComponent_SetRotation(ulong eID, ref Quaternion rotation);
        #endregion

        #region GameObject
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static string GameObject_GetName(ulong eID);
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void GameObject_SetName(ulong eID, string type);
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static bool GameObject_HasComponent(ulong eID, ref Type type);
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static ulong GameObject_GetComponent(ulong eID, ref Type type);
        #endregion

    }
}