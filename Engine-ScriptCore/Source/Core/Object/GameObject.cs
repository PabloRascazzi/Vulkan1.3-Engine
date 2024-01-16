using System;

namespace Engine {

    public class GameObject {

        private readonly ulong ID;

        internal GameObject(ulong ID) { 
            this.ID = ID;
        }

        public string name {
            get => InternalCalls.GameObject_GetName(this.ID);
            set => InternalCalls.GameObject_SetName(this.ID, value);
        }

        public bool HasComponent<T>() where T : Component, new() {
            Type componentType = typeof(T);
            return InternalCalls.GameObject_HasComponent(this.ID, ref componentType);
        }

        public T GetComponent<T>() where T : Component, new() {
            Type componentType = typeof(T);
            ulong cID = InternalCalls.GameObject_GetComponent(this.ID, ref componentType);
            return new T() { gameObject = this, ComponentID = cID };
        }
    }
}
