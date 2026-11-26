public class ConstantPropagation {
    public static void main(String[] args) {
        StaticJavaLib.println(compute());
    }

    static int compute() {
        int base;
        int doubled;

        base = 6;
        doubled = base * 2;
        if (doubled > 10) {
            return doubled + 1;
        } else {
            return 0;
        }
    }
}
