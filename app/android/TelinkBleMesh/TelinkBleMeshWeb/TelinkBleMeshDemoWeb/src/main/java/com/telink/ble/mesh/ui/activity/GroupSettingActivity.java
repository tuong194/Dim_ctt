/********************************************************************************************************
 * @file GroupSettingActivity.java
 *
 * @brief for TLSR chips
 *
 * @author telink
 * @date Sep. 30, 2017
 *
 * @par Copyright (c) 2017, Telink Semiconductor (Shanghai) Co., Ltd. ("TELINK")
 *
 *          Licensed under the Apache License, Version 2.0 (the "License");
 *          you may not use this file except in compliance with the License.
 *          You may obtain a copy of the License at
 *
 *              http://www.apache.org/licenses/LICENSE-2.0
 *
 *          Unless required by applicable law or agreed to in writing, software
 *          distributed under the License is distributed on an "AS IS" BASIS,
 *          WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *          See the License for the specific language governing permissions and
 *          limitations under the License.
 *******************************************************************************************************/
//package com.telink.ble.mesh.ui;
//
//import android.content.Intent;
//import android.os.Bundle;
//import android.view.View;
//import android.widget.SeekBar;
//import android.widget.TextView;
//
//import androidx.recyclerview.widget.GridLayoutManager;
//import androidx.recyclerview.widget.RecyclerView;
//
//import com.telink.ble.mesh.TelinkMeshApplication;
//import com.telink.ble.mesh.core.message.MeshMessage;
//import com.telink.ble.mesh.core.message.generic.OnOffSetMessage;
//import com.telink.ble.mesh.core.message.lighting.CtlTemperatureSetMessage;
//import com.telink.ble.mesh.core.message.lighting.LightnessSetMessage;
//import com.telink.ble.mesh.demo.R;
//import com.telink.ble.mesh.foundation.Event;
//import com.telink.ble.mesh.foundation.EventListener;
//import com.telink.ble.mesh.foundation.MeshService;
//import com.telink.ble.mesh.foundation.event.MeshEvent;
//import com.telink.ble.mesh.model.AppSettings;
//import com.telink.ble.mesh.model.GroupInfo;
//import com.telink.ble.mesh.model.MeshInfo;
//import com.telink.ble.mesh.model.NodeInfo;
//import com.telink.ble.mesh.model.NodeStatusChangedEvent;
//import com.telink.ble.mesh.model.UnitConvert;
//import com.telink.ble.mesh.ui.adapter.BaseRecyclerViewAdapter;
//import com.telink.ble.mesh.ui.adapter.OnlineDeviceListAdapter;
//import com.telink.ble.mesh.util.MeshLogger;
//import com.telink.ble.mesh.web.entity.MeshNode;
//
//import java.util.ArrayList;
//import java.util.List;
//
///**
// * Group Settings : lum / temp control
// * Created by kee on 2017/8/30.
// */
//
//public class GroupSettingActivity extends BaseActivity implements EventListener<String> {
//
//    private OnlineDeviceListAdapter mAdapter;
//
//    private SeekBar lum, temp;
//    private TextView tv_lum, tv_temp;
//    private RecyclerView rv_groups;
//    private GroupInfo group;
//
//    private SeekBar.OnSeekBarChangeListener onProgressChangeListener = new SeekBar.OnSeekBarChangeListener() {
//
//        private long preTime;
//        private static final int DELAY_TIME = 320;
//
//
//        @Override
//        public void onProgressChanged(SeekBar seekBar, int progress, boolean fromUser) {
//            onProgressUpdate(seekBar, progress, false);
//        }
//
//        @Override
//        public void onStartTrackingTouch(SeekBar seekBar) {
//
//        }
//
//        @Override
//        public void onStopTrackingTouch(SeekBar seekBar) {
//            onProgressUpdate(seekBar, seekBar.getProgress(), true);
//        }
//
//        private void onProgressUpdate(SeekBar seekBar, int progress, boolean immediate) {
//            long currentTime = System.currentTimeMillis();
//            if ((currentTime - this.preTime) >= DELAY_TIME || immediate) {
//                this.preTime = currentTime;
//
//                MeshInfo meshInfo = TelinkMeshApplication.getInstance().getMeshInfo();
//                MeshMessage meshMessage;
//                if (seekBar == lum) {
//                    progress = Math.max(1, progress);
//                    MeshLogger.d(("lum: " + progress + " -- lightness: " + UnitConvert.lum2lightness(progress)));
//                    meshMessage = LightnessSetMessage.getSimple(group.address,
//                            meshInfo.getDefaultAppKeyIndex(),
//                            UnitConvert.lum2lightness(progress),
//                            false, 0);
//                    MeshService.getInstance().sendMeshMessage(meshMessage);
//                    tv_lum.setText(getString(R.string.lum_progress, progress, Integer.toHexString(group.address)));
//                } else if (seekBar == temp) {
//                    meshMessage = CtlTemperatureSetMessage.getSimple(group.address,
//                            meshInfo.getDefaultAppKeyIndex(), UnitConvert.temp100ToTemp(progress),
//                            0, false, 0);
//                    MeshService.getInstance().sendMeshMessage(meshMessage);
//                    tv_temp.setText(getString(R.string.temp_progress, progress, Integer.toHexString(group.address)));
//                }
//            } else {
//                MeshLogger.w("CMD reject: " + progress);
//            }
//        }
//    };
//
//    @Override
//    protected void onCreate(Bundle savedInstanceState) {
//        super.onCreate(savedInstanceState);
//        if (!validateNormalStart(savedInstanceState)) {
//            return;
//        }
//        setContentView(R.layout.activity_group_setting);
//
//        final Intent intent = getIntent();
//        if (intent.hasExtra("group")) {
//            group = (GroupInfo) intent.getSerializableExtra("group");
//        } else {
//            toastMsg("group null");
//            finish();
//            return;
//        }
//
//        TextView tv_group_name = findViewById(R.id.tv_group_name);
//        tv_group_name.setText(group.name + ":");
//        lum = findViewById(R.id.sb_brightness);
//        temp = findViewById(R.id.sb_temp);
//
//        rv_groups = findViewById(R.id.rv_device);
//
//        setTitle("Group Setting");
//        enableBackNav(true);
//        final List<MeshNode> innerDevices = getDevicesInGroup();
//        mAdapter = new OnlineDeviceListAdapter(this, innerDevices);
//        mAdapter.setOnItemClickListener(new BaseRecyclerViewAdapter.OnItemClickListener() {
//            @Override
//            public void onItemClick(int position) {
//                if (innerDevices.get(position).isOffline()) return;
//
//                byte onOff = 0;
//                if (innerDevices.get(position).isOff()) {
//                    onOff = 1;
//                }
//                int address = innerDevices.get(position).meshAddress;
//
//                int appKeyIndex = TelinkMeshApplication.getInstance().getMeshInfo().getDefaultAppKeyIndex();
//                OnOffSetMessage onOffSetMessage = OnOffSetMessage.getSimple(address, appKeyIndex, onOff, !AppSettings.ONLINE_STATUS_ENABLE, !AppSettings.ONLINE_STATUS_ENABLE ? 1 : 0);
//                MeshService.getInstance().sendMeshMessage(onOffSetMessage);
//            }
//        });
//
//        rv_groups.setLayoutManager(new GridLayoutManager(this, 3));
//        rv_groups.setAdapter(mAdapter);
//
//        findViewById(R.id.tv_color).setOnClickListener(new View.OnClickListener() {
//            @Override
//            public void onClick(View v) {
//                Intent colorIntent = new Intent(GroupSettingActivity.this, ColorPanelActivity.class);
//                colorIntent.putExtra("address", group.address);
//                startActivity(colorIntent);
//            }
//        });
//        lum.setEnabled(innerDevices.size() != 0);
//        temp.setEnabled(innerDevices.size() != 0);
//
//        tv_lum = findViewById(R.id.tv_lum);
//        tv_temp = findViewById(R.id.tv_temp);
//        tv_lum.setText(getString(R.string.lum_progress, 10, Integer.toHexString(group.address)));
//        tv_temp.setText(getString(R.string.temp_progress, 10, Integer.toHexString(group.address)));
//
//        lum.setOnSeekBarChangeListener(this.onProgressChangeListener);
//
//        temp.setOnSeekBarChangeListener(this.onProgressChangeListener);
//
//        TelinkMeshApplication.getInstance().addEventListener(NodeStatusChangedEvent.EVENT_TYPE_NODE_STATUS_CHANGED, this);
//        TelinkMeshApplication.getInstance().addEventListener(MeshEvent.EVENT_TYPE_DISCONNECTED, this);
//    }
//
//    private List<MeshNode> getDevicesInGroup() {
//
//        List<MeshNode> localDevices = TelinkMeshApplication.getInstance().getMeshInfo().nodeList;
//        List<MeshNode> innerDevices = new ArrayList<>();
//        outer:
//        for (MeshNode device : localDevices) {
//            if (device.subList != null) {
//                for (int groupAdr : device.subList) {
//                    if (groupAdr == group.address) {
//                        innerDevices.add(device);
//                        continue outer;
//                    }
//                }
//            }
//        }
//        return innerDevices;
//    }
//
//    private void refreshUI() {
//        runOnUiThread(new Runnable() {
//            @Override
//            public void run() {
//                mAdapter.notifyDataSetChanged();
//            }
//        });
//    }
//
//    @Override
//    public void performed(Event<String> event) {
//        if (event.getType().equals(MeshEvent.EVENT_TYPE_DISCONNECTED)
//                || event.getType().equals(NodeStatusChangedEvent.EVENT_TYPE_NODE_STATUS_CHANGED)) {
//            refreshUI();
//        }
//    }
//}
