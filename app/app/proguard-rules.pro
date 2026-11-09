# Firebase Realtime Database Proguard Rules
-keepattributes Signature
-keepattributes *Annotation*
-keep class com.google.firebase.database.** { *; }
-keepclassmembers class * {
    @com.google.firebase.database.PropertyName <fields>;
}
-keep class com.agribridge.app.data.model.** { *; }
