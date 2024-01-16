namespace Engine {

    public abstract partial class Script : Component {

        protected virtual void Start() { }
        protected virtual void Update() { }
        protected virtual void OnDestroy() { }

    }
}
