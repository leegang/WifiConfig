<template>
  <div class="max-w-md mx-auto p-4 bg-white">
    <h1 class="page-heading">Configuration Manager</h1>

    <div class="mt-8">
      <div class="flex flex-row items-center border-b mb-4">
        <router-link
          to="/home"
          class="tab-selector"
          active-class="--selected"
        >
          <v-icon icon="wifi-solid" class="w-5 h-5 mr-2" />
          WiFi
        </router-link>
        <router-link
          to="/set"
          class="tab-selector"
          active-class="--selected"
        >
          <v-icon icon="cog-solid" class="w-5 h-5 mr-2" />
          Settings
        </router-link>
        <router-link
          to="/status"
          class="tab-selector"
          active-class="--selected"
        >
          Manage
        </router-link>
      </div>
      <router-view></router-view>
      <!-- <v-wifi v-if="selectedTab === 1"/>
            <v-settings v-if="selectedTab === 2" />
            <v-status v-if="selectedTab === 3" /> -->
    </div>
  </div>
</template>

<script>
import VueRouter from "vue-router";
import Vue from "vue";

import Status from "./Status.vue";
import WiFi from "./WiFi.vue";
import Settings from "./Settings.vue";

const routers = [
      {
        path: "/home",
        name: "WiFi",
        component: WiFi,
      },
      {
        path: "/set",
        name: "Settings",
        component: Settings,
      },
      {
        path: "/status",
        name: "Status",
        component: Status,
      },
      {
        path: "/",
        redirect:"/home"
      },
    ];
Vue.use(VueRouter);
const router = new VueRouter({
    mode :"history",
    routes : routers
});


export default {
    router:router,
    components: {
      "v-status": Status,
      "v-wifi": WiFi,
      "v-settings": Settings,
    },
    
  };
</script>

<style lang="scss">
.tab-selector {
  @apply inline-flex items-center py-1 px-2 bg-white text-sm;

  &.--selected {
    @apply bg-gray-100 text-primary;
  }
}
</style>