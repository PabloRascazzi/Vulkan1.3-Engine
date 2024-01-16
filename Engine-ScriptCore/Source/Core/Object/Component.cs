using System;

namespace Engine {

    public abstract class Component {

        public GameObject gameObject { get; internal set; }
        public ulong ComponentID { get; internal set; }

        public bool HasComponent<T>() where T : Component, new() {
            return gameObject.HasComponent<T>();
        }

        public T GetComponent<T>() where T : Component, new() {
            return gameObject.GetComponent<T>();
        }
    }
}
